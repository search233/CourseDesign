#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "core/frame_builder.h"
#include "core/utils.h"

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace ftxui;
using namespace ethernet;

int main() {
    auto screen = ScreenInteractive::Fullscreen();

    // ==========================================
    // 1. 状态定义 (State)
    // ==========================================
    std::string input_dst_mac = "AA:BB:CC:DD:EE:FF";
    std::string input_src_mac = "00:11:22:33:44:55";
    std::string input_payload = "Hello 802.3";

    Result current_result;
    bool has_run = false;
    size_t current_step = 0;

    // --- 播放状态与线程控制 ---
    std::atomic<bool> is_playing{false};
    std::atomic<bool> app_running{true};
    int speed_ms = 150;                  // 默认步进间隔 150ms
    std::atomic<int> atomic_speed_ms{150};
    std::string play_label = " ▶ 播放 ";

    // ==========================================
    // 2. 交互逻辑定义 (Components)
    // ==========================================
    Component comp_dst = Input(&input_dst_mac, "AA:BB:CC:DD:EE:FF");
    Component comp_src = Input(&input_src_mac, "00:11:22:33:44:55");
    Component comp_payload = Input(&input_payload, "输入待发送数据...");

    // 封装按钮
    auto on_encapsulate = [&] {
        is_playing = false;
        play_label = " ▶ 播放 ";
        current_result = builder::encapsulate(input_dst_mac, input_src_mac, input_payload);
        has_run = true;
        current_step = 0;
    };
    Component btn_run = Button(" 执行封装 (Enter) ", on_encapsulate);

    // CRC 控制按钮集
    Component btn_prev = Button(" < 上一步 ", [&] {
        is_playing = false;
        play_label = " ▶ 播放 ";
        if (current_step > 0) current_step--;
    });

    Component btn_next = Button(" 下一步 > ", [&] {
        is_playing = false;
        play_label = " ▶ 播放 ";
        if (has_run && current_result.is_ok() && current_step + 1 < current_result.crc_snaps.size()) {
            current_step++;
        }
    });

    Component btn_play = Button(&play_label, [&] {
        if (!has_run || !current_result.is_ok() || current_result.crc_snaps.empty()) return;
        // 若已处于终点，再次点击播放则从第一步重新开始
        if (!is_playing && current_step + 1 >= current_result.crc_snaps.size()) {
            current_step = 0;
        }
        is_playing = !is_playing;
        play_label = is_playing ? " ⏸ 暂停 " : " ▶ 播放 ";
    });

    Component btn_reset = Button(" ↺ 重置 ", [&] {
        is_playing = false;
        play_label = " ▶ 播放 ";
        current_step = 0;
    });

    // 播放速度滑块：20ms ~ 600ms
    SliderOption<int> slider_opt;
    slider_opt.value = &speed_ms;
    slider_opt.min = 20;
    slider_opt.max = 600;
    slider_opt.increment = 20;
    slider_opt.color_active = Color::Cyan;
    slider_opt.color_inactive = Color::GrayDark;
    Component slider_speed = Slider(slider_opt);

    // 容器焦点树
    auto layout_container = Container::Vertical({
        comp_dst,
        comp_src,
        comp_payload,
        btn_run,
        Container::Horizontal({
            btn_prev,
            btn_play,
            btn_next,
            btn_reset,
            slider_speed
        })
    });

    // ==========================================
    // 3. 界面渲染流水线 (Renderer)
    // ==========================================
    auto renderer = Renderer(layout_container, [&] {
        // 同步速度到原子变量供后台线程读取
        atomic_speed_ms.store(speed_ms);

        // --- 模块 A: 输入区域 ---
        auto input_box = vbox({
            hbox({ text(" 目的 MAC: ") | bold, comp_dst->Render() | flex }),
            hbox({ text(" 源  MAC:  ") | bold, comp_src->Render() | flex }),
            hbox({ text(" 数据载荷: ") | bold, comp_payload->Render() | flex }),
            separator(),
            hbox({ filler(), btn_run->Render(), filler() })
        }) | border | color(Color::Cyan);

        // --- 模块 B: 错误提示栏 ---
        Element status_banner = text("");
        if (has_run) {
            if (!current_result.is_ok()) {
                status_banner = text(" [错误] " + current_result.error_message) | color(Color::Red) | bold;
            } else {
                status_banner = text(" [成功] 封装完成! 原始数据: " + 
                                     std::to_string(current_result.frame.payload_length) + "B, 填充: " + 
                                     std::to_string(current_result.frame.padding.size()) + "B") | color(Color::Green);
            }
        }

        // --- 模块 C: 彩色 Hex Dump 预览 (窄版侧边栏) ---
        Elements hex_elements;
        if (has_run && current_result.is_ok()) {
            auto raw_bytes = current_result.frame.serialize();
            auto spans = current_result.frame.get_field_spans();

            for (size_t i = 0; i < raw_bytes.size(); ++i) {
                Color byte_color = Color::White;
                for (const auto& span : spans) {
                    if (i >= span.offset && i < span.offset + span.length) {
                        switch (span.type) {
                            case FrameSegmentType::Preamble: byte_color = Color::Green; break;
                            case FrameSegmentType::SFD:      byte_color = Color::GreenLight; break;
                            case FrameSegmentType::DestMac:  byte_color = Color::BlueLight; break;
                            case FrameSegmentType::SrcMac:   byte_color = Color::Cyan; break;
                            case FrameSegmentType::Length:   byte_color = Color::Yellow; break;
                            case FrameSegmentType::Payload:  byte_color = Color::White; break;
                            case FrameSegmentType::Padding:  byte_color = Color::GrayLight; break;
                            case FrameSegmentType::FCS:      byte_color = Color::RedLight; break;
                        }
                        break;
                    }
                }
                std::stringstream ss;
                ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)raw_bytes[i];
                hex_elements.push_back(text(ss.str() + " ") | color(byte_color));
            }
        }

        auto hex_box = vbox({
            text(" 物理帧 (Hex Dump) ") | bold | hcenter,
            separator(),
            has_run && current_result.is_ok() 
                ? hflow(std::move(hex_elements)) 
                : text("请先执行封装...") | dim,
            filler(),
            separator(),
            text("图例说明:") | bold | color(Color::White),
            hbox({ text("■ 前导 ") | color(Color::Green),      text("■ SFD ") | color(Color::GreenLight) }),
            hbox({ text("■ 目的MAC ") | color(Color::BlueLight), text("■ 源MAC ") | color(Color::Cyan) }),
            hbox({ text("■ 长度 ") | color(Color::Yellow),     text("■ 数据 ") | color(Color::White) }),
            hbox({ text("■ 填充 ") | color(Color::GrayLight),  text("■ FCS ") | color(Color::RedLight) })
        }) | border | size(WIDTH, EQUAL, 40); // 锁定紧凑宽度为 40 列

        // --- 模块 D: CRC 模 2 演算步进与播放控制器 ---
        Element crc_detail = text("请先执行封装以查看模 2 除法过程...") | dim;
        if (has_run && current_result.is_ok() && !current_result.crc_snaps.empty()) {
            const auto& snap = current_result.crc_snaps[current_step];
            size_t crc_bytes = current_result.frame.destmac.size() + current_result.frame.srcmac.size() + 2 + current_result.frame.payload.size() + current_result.frame.padding.size();
            
            crc_detail = vbox({
                text("生成多项式 G(X): 1 0 0 0 0 0 1 1 1 (CRC-8 / 0x107)") | color(Color::Yellow) | bold,
                text("校验范围 (IEEE 802.3): 目的MAC(6B) + 源MAC(6B) + 长度(2B) + 载荷(" + 
                     std::to_string(current_result.frame.payload.size()) + "B) + 填充(" + 
                     std::to_string(current_result.frame.padding.size()) + "B) = " + 
                     std::to_string(crc_bytes) + "B (" + 
                     std::to_string(crc_bytes * 8) + " 比特 / 共 " + 
                     std::to_string(current_result.crc_snaps.size()) + " 步)") | color(Color::CyanLight),
                separator(),
                hbox({ text("当前步数:     ") | dim, text("第 " + std::to_string(current_step + 1) + " 步 / 共 " + std::to_string(current_result.crc_snaps.size()) + " 步") | bold | color(Color::Cyan) }),
                hbox({ text("被除数切片:     ") | dim, text(snap.current_window) | bold | color(Color::White) }),
                snap.is_xor 
                    ? hbox({ text("多项式运算:   ") | dim, text("⊕ 100000111  (首位为1, 执行模 2 异或 XOR)") | color(Color::RedLight) | bold })
                    : hbox({ text("多项式运算:   ") | dim, text("- ---------  (首位为0, 仅直移不异或)") | color(Color::GrayLight) }),
                hbox({ text("本步所得余数:   ") | dim, text(snap.remainder) | bold | color(Color::GreenLight) }),
                hbox({ text("下一比特滑入: ") | dim, 
                       (snap.next_bit == '\0' 
                            ? text("无 (运算完毕)") | color(Color::Yellow) | bold 
                            : text(std::string(1, snap.next_bit)) | bold | color(Color::White)) }),
                separator(),
                current_step + 1 == current_result.crc_snaps.size()
                    ? hbox({ text("★ 模 2 除法演算完毕! 最终余数 (FCS): ") | bold | color(Color::GreenLight),
                            text(snap.remainder.substr(1)) | bold | color(Color::Yellow),
                            text(" (0x" + utils::byte_to_hex(static_cast<uint8_t>(current_result.frame.fcs & 0xFF)) + ")") | bold | color(Color::Yellow) })
                    : hbox({ text("提示: 点击 [下一步] 或 [播放] 查看滑动除法过程") | dim })
            });
        }

        auto crc_control_bar = hbox({
            text(" 步数: " + (has_run && current_result.is_ok() ? 
                std::to_string(current_step + 1) + "/" + std::to_string(current_result.crc_snaps.size()) : "0/0") + " ") | bold | color(Color::Cyan) | vcenter,
            btn_prev->Render(),
            btn_play->Render(),
            btn_next->Render(),
            btn_reset->Render(),
            separator(),
            text(" ⏱ 播放延时: ") | bold | color(Color::Yellow) | vcenter,
            slider_speed->Render() | size(WIDTH, EQUAL, 16) | vcenter,
            text(" " + std::to_string(speed_ms) + " ms ") | bold | color(Color::Black) | bgcolor(Color::Yellow) | vcenter
        });

        auto crc_box = vbox({
            text(" CRC 模 2 除法过程追踪 ") | bold,
            separator(),
            crc_control_bar,
            separator(),
            crc_detail | flex
        }) | border | color(Color::Magenta) | flex; // 撑满右侧剩余宽度

        // --- 水平分栏主体 ---
        auto main_split = hbox({
            hex_box,
            separator(),
            crc_box
        }) | flex;

        // --- 全屏总排版 ---
        return vbox({
            text(" IEEE 802.3 帧封装与 CRC 可视化仿真器 ") | bold | hcenter | color(Color::Yellow),
            status_banner | hcenter,
            input_box,
            main_split
        });
    });

    // ==========================================
    // 4. 事件监听与后台自动播放线程
    // ==========================================
    auto main_component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Character('q') || event == Event::Escape) {
            app_running = false;
            screen.ExitLoopClosure()();
            return true;
        }

        // 响应后台线程发送的计时器自定义事件
        if (event == Event::Custom) {
            if (is_playing && has_run && current_result.is_ok()) {
                if (current_step + 1 < current_result.crc_snaps.size()) {
                    current_step++;
                } else {
                    // 播放完毕自动停止
                    is_playing = false;
                    play_label = " ▶ 播放 ";
                }
            }
            return true;
        }
        return false;
    });

    // 启动非阻塞播放时钟线程
    std::thread play_thread([&] {
        while (app_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(atomic_speed_ms.load()));
            if (is_playing.load()) {
                screen.PostEvent(Event::Custom);
            }
        }
    });

    // 运行主循环
    screen.Loop(main_component);

    // 退出时优雅回收后台线程
    app_running = false;
    if (play_thread.joinable()) {
        play_thread.join();
    }

    return 0;
}