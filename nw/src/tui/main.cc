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
    Component slider_speed = Slider(" 延时: ", &speed_ms, 20, 600, 20);

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
                            case FrameSegmentType::DestMac:  byte_color = Color::Blue; break;
                            case FrameSegmentType::SrcMac:   byte_color = Color::Cyan; break;
                            case FrameSegmentType::Length:   byte_color = Color::Yellow; break;
                            case FrameSegmentType::Payload:  byte_color = Color::White; break;
                            case FrameSegmentType::Padding:  byte_color = Color::GrayDark; break;
                            case FrameSegmentType::FCS:      byte_color = Color::Red; break;
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
            text("图例:") | bold,
            text("[前导/SFD 绿]") | color(Color::Green),
            text("[MAC 蓝/青]") | color(Color::Cyan),
            text("[长度 黄] [数据 白]") | color(Color::Yellow),
            text("[填充 灰] [FCS 红]") | color(Color::Red)
        }) | border | size(WIDTH, EQUAL, 40); // 锁定紧凑宽度为 40 列

        // --- 模块 D: CRC 模 2 演算步进与播放控制器 ---
        Element crc_detail = text("无运算数据") | dim;
        if (has_run && current_result.is_ok() && !current_result.crc_snaps.empty()) {
            const auto& snap = current_result.crc_snaps[current_step];
            
            crc_detail = vbox({
                text("生成多项式 G(X): 1 0 0 0 0 0 1 1 1 (CRC-8)") | color(Color::Yellow),
                separator(),
                text("被除数切片:   " + snap.current_window) | bold,
                snap.is_xor 
                    ? text("多项式异或: ⊕ 100000111  (首位为1, 执行XOR)") | color(Color::Red)
                    : text("直接左移:   - ---------  (首位为0, 仅移位)") | color(Color::GrayDark),
                text("本步余数:     " + snap.remainder) | color(Color::Green),
                text("下一位移入:   " + (snap.next_bit == '\0' ? std::string("(结束)") : std::string(1, snap.next_bit))) | dim
            });
        }

        auto crc_control_bar = hbox({
            text(" 步数: " + (has_run && current_result.is_ok() ? 
                std::to_string(current_step + 1) + "/" + std::to_string(current_result.crc_snaps.size()) : "0/0") + " ") | bold | ycenter,
            btn_prev->Render(),
            btn_play->Render(),
            btn_next->Render(),
            btn_reset->Render(),
            separator(),
            slider_speed->Render() | size(WIDTH, EQUAL, 18),
            text(" " + std::to_string(speed_ms) + "ms ") | dim | ycenter
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