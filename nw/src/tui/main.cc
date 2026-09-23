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
#include <algorithm>

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
                hex_elements.push_back(text(utils::byte_to_hex(raw_bytes[i]) + " ") | color(byte_color));
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

        // --- 模块 D: CRC 模 2 演算步进与播放控制器 (字节/比特向左平滑流动，固定计算工位) ---
        Element crc_detail = text("请先执行封装以查看模 2 除法过程...") | dim;
        if (has_run && current_result.is_ok() && !current_result.crc_snaps.empty()) {
            const auto& snap = current_result.crc_snaps[current_step];
            const auto& frame = current_result.frame;
            auto crc_payload = frame.get_crc_payload();
            size_t payload_len = frame.payload.size();
            size_t padding_len = frame.padding.size();

            // 参与 CRC 模 2 除法的所有字节 (原始数据 + 8个零比特形成的附加字节)
            int total_crc_bytes = static_cast<int>(crc_payload.size()) + 1;
            int current_byte = static_cast<int>(current_step) / 8;
            size_t total_steps = current_result.crc_snaps.size();

            // 辅助：获取任意字节的归属字段名称、色彩与字节数值
            auto get_byte_info = [&](int b, std::string& field_name, Color& field_color, uint8_t& val) {
                val = (b < static_cast<int>(crc_payload.size())) ? crc_payload[b] : 0x00;
                if (b < 6) {
                    field_name = "DestMAC[" + std::to_string(b) + "]";
                    field_color = Color::BlueLight;
                } else if (b < 12) {
                    field_name = "SrcMAC[" + std::to_string(b - 6) + "]";
                    field_color = Color::Cyan;
                } else if (b == 12) {
                    field_name = "Length[Hi]";
                    field_color = Color::Yellow;
                } else if (b == 13) {
                    field_name = "Length[Lo]";
                    field_color = Color::Yellow;
                } else if (b < 14 + static_cast<int>(payload_len)) {
                    field_name = "Data[" + std::to_string(b - 14) + "]";
                    field_color = Color::White;
                } else if (b < 14 + static_cast<int>(payload_len + padding_len)) {
                    field_name = "Pad[" + std::to_string(b - 14 - payload_len) + "]";
                    field_color = Color::GrayLight;
                } else {
                    field_name = "+Zeros(FCS)";
                    field_color = Color::RedLight;
                }
            };

            // 辅助：居中填充文本至指定字符宽度 (纯 ASCII)
            auto center_text = [](const std::string& s, int width) -> std::string {
                if (static_cast<int>(s.length()) >= width) return s.substr(0, width);
                int left = (width - static_cast<int>(s.length())) / 2;
                int right = width - static_cast<int>(s.length()) - left;
                return std::string(left, ' ') + s + std::string(right, ' ');
            };

            // 终端宽度与画布尺寸计算
            int term_width = Terminal::Size().dimx;
            if (term_width <= 0) term_width = 120;
            int canvas_width = std::max(64, term_width - 40 - 22);

            // 固定计算工位锚点 (ANCHOR_COL):
            // 计算窗口首位与多项式永远锚定在第 16 列；
            // 左侧 0~15 列展示已处理并向左移出的 1 个历史字节 (8 比特)；
            // 随着 step 递增，所有字节与比特整体向左平滑移入/移出。
            const int ANCHOR_COL = 16;
            int window_start_bit = static_cast<int>(current_step);
            int window_end_bit = window_start_bit + 8; // 9 位计算窗口

            // 1. 宏观字段归属与进度条
            std::string cur_field_desc;
            Color cur_field_color = Color::White;
            if (current_byte < 6) {
                cur_field_desc = "目的 MAC 地址 (DestMAC)";
                cur_field_color = Color::BlueLight;
            } else if (current_byte < 12) {
                cur_field_desc = "源 MAC 地址 (SrcMAC)";
                cur_field_color = Color::Cyan;
            } else if (current_byte < 14) {
                cur_field_desc = "长度字段 (Length)";
                cur_field_color = Color::Yellow;
            } else if (current_byte < 14 + static_cast<int>(payload_len)) {
                cur_field_desc = "数据载荷 (Payload, 字节 " + std::to_string(current_byte - 14 + 1) + "/" + std::to_string(payload_len) + ")";
                cur_field_color = Color::White;
            } else if (current_byte < 14 + static_cast<int>(payload_len + padding_len)) {
                cur_field_desc = "补齐填充 (Padding, 字节 " + std::to_string(current_byte - 14 - payload_len + 1) + "/" + std::to_string(padding_len) + ")";
                cur_field_color = Color::GrayLight;
            } else {
                cur_field_desc = "校验填充零比特 (+8 Zeros)";
                cur_field_color = Color::RedLight;
            }

            float progress_val = static_cast<float>(current_step + 1) / static_cast<float>(total_steps);
            auto macro_progress_bar = hbox({
                text(" 全局进度: ") | bold,
                gauge(progress_val) | color(Color::Cyan) | size(WIDTH, EQUAL, 20),
                text(" " + std::to_string(current_step + 1) + "/" + std::to_string(total_steps) + " 步") | bold | color(Color::CyanLight),
                text(" │ 当前阶段: " + cur_field_desc) | color(cur_field_color) | bold,
                text(" │ 实际 9 位: ") | dim,
                text(snap.current_window) | bold | color(Color::Yellow)
            });

            // 辅助通用构造向左滑动的字节级标头行 (16进制行)
            auto build_sliding_header_row = [&](const std::string& row_label, auto get_text_fn, Color default_color, bool use_field_color) {
                Elements row_elements;
                int cur_col = 0;

                for (int b = 0; b < total_crc_bytes; ++b) {
                    int b_col = ANCHOR_COL + (b * 8 - static_cast<int>(current_step)) * 2;
                    int b_end = b_col + 16;

                    // 完全在可视区域左侧
                    if (b_end <= 0) continue;
                    // 完全在可视区域右侧
                    if (b_col >= canvas_width) break;

                    // 填充前置间隙
                    if (b_col > cur_col) {
                        row_elements.push_back(text(std::string(b_col - cur_col, ' ')));
                        cur_col = b_col;
                    }

                    int start_in_s = std::max(0, -b_col);
                    int end_in_s = std::min(16, canvas_width - b_col);
                    int len = end_in_s - start_in_s;

                    std::string fname;
                    Color fcolor;
                    uint8_t fval;
                    get_byte_info(b, fname, fcolor, fval);

                    std::string full_cell = get_text_fn(b, fname, fval);
                    std::string slice = (len > 0 && start_in_s < static_cast<int>(full_cell.length()))
                        ? full_cell.substr(start_in_s, len)
                        : "";

                    Color c = use_field_color ? fcolor : default_color;
                    row_elements.push_back(text(slice) | color(c) | bold);
                    cur_col += len;
                }

                return hbox({
                    text(row_label) | dim | size(WIDTH, EQUAL, 12),
                    separator(),
                    hbox(std::move(row_elements))
                });
            };

            // 2. 行 1: 十六进制数值标注 (向左流动)
            auto row_hex = build_sliding_header_row(
                " 16 进制  ",
                [&](int, const std::string&, uint8_t fval) {
                    return center_text("[ 0x" + utils::byte_to_hex(fval) + " ]", 16);
                },
                Color::Yellow,
                false
            );

            // 3. 行 2: 连续二进制比特串 (向左平滑滑动，进入工位窗口高亮)
            Elements row3_elements;
            int cur_col = 0;
            int total_bits = total_crc_bytes * 8;

            for (int g = 0; g < total_bits; ++g) {
                int g_col = ANCHOR_COL + (g - static_cast<int>(current_step)) * 2;
                int g_end = g_col + 2;

                if (g_end <= 0) continue;
                if (g_col >= canvas_width) break;

                if (g_col > cur_col) {
                    row3_elements.push_back(text(std::string(g_col - cur_col, ' ')));
                    cur_col = g_col;
                }

                int byte_idx = g / 8;
                int bit_offset = 7 - (g % 8);
                uint8_t b_val = (byte_idx < static_cast<int>(crc_payload.size())) ? crc_payload[byte_idx] : 0x00;
                char bit_ch = ((b_val >> bit_offset) & 1) ? '1' : '0';

                Color bit_color = Color::White;
                bool is_bold = false;
                bool is_dim = false;

                if (g < window_start_bit) {
                    bit_color = Color::GrayDark;
                    is_dim = true;
                } else if (g >= window_start_bit && g <= window_end_bit) {
                    is_bold = true;
                    bit_color = snap.is_xor ? Color::RedLight : Color::CyanLight;
                } else {
                    bit_color = Color::White;
                }

                int start_in_b = std::max(0, -g_col);
                int end_in_b = std::min(2, canvas_width - g_col);
                int len = end_in_b - start_in_b;

                std::string bit_str = std::string(1, bit_ch) + " ";
                std::string slice = bit_str.substr(start_in_b, len);

                Element elem = text(slice) | color(bit_color);
                if (is_bold) elem = elem | bold;
                if (is_dim) elem = elem | dim;
                row3_elements.push_back(elem);
                cur_col += len;
            }

            auto row3 = hbox({
                text(" 比特数据 ") | bold | color(Color::Cyan) | size(WIDTH, EQUAL, 12),
                separator(),
                hbox(std::move(row3_elements))
            });

            // 4. 行 3: 实际参与计算的 9 位被除数 (固定锚定在 ANCHOR_COL 工位)
            Elements row_actual_elements;
            if (ANCHOR_COL > 0) {
                row_actual_elements.push_back(text(std::string(ANCHOR_COL, ' ')));
            }
            std::string actual_9bits_spaced;
            for (size_t i = 0; i < snap.current_window.size(); ++i) {
                actual_9bits_spaced.push_back(snap.current_window[i]);
                actual_9bits_spaced.push_back(' ');
            }
            if (!actual_9bits_spaced.empty()) {
                actual_9bits_spaced.pop_back(); // 去掉末尾空格，正好 17 列
            }

            Color actual_color = snap.is_xor ? Color::RedLight : Color::CyanLight;
            row_actual_elements.push_back(text(actual_9bits_spaced) | bold | color(actual_color));

            auto row_actual = hbox({
                text(" 实际9位  ") | bold | color(Color::Yellow) | size(WIDTH, EQUAL, 12),
                separator(),
                hbox(std::move(row_actual_elements))
            });

            // 5. 行 4: 模 2 多项式除数 (固定锚定在 ANCHOR_COL 工位)
            Elements row4_elements;
            if (ANCHOR_COL > 0) {
                row4_elements.push_back(text(std::string(ANCHOR_COL, ' ')));
            }
            if (snap.is_xor) {
                row4_elements.push_back(text("1 0 0 0 0 0 1 1 1") | bold | color(Color::RedLight));
                row4_elements.push_back(text("  (首位为1: 模 2 异或 ⊕)") | bold | color(Color::RedLight));
            } else {
                row4_elements.push_back(text("- - - - - - - - -") | color(Color::GrayLight));
                row4_elements.push_back(text("  (首位为0: 仅直移不异或)") | color(Color::GrayLight));
            }
            auto row4 = hbox({
                text(" 多项式   ") | color(Color::Yellow) | size(WIDTH, EQUAL, 12),
                separator(),
                hbox(std::move(row4_elements))
            });

            // 6. 行 4.5: 算式横线 (固定锚定在 ANCHOR_COL 工位)
            Elements row4_5_elements;
            if (ANCHOR_COL > 0) {
                row4_5_elements.push_back(text(std::string(ANCHOR_COL, ' ')));
            }
            row4_5_elements.push_back(text("─────────────────") | color(Color::GrayDark));
            auto row4_5 = hbox({
                text(" ──────── ") | color(Color::GrayDark) | size(WIDTH, EQUAL, 12),
                separator(),
                hbox(std::move(row4_5_elements))
            });

            // 7. 行 5: 本步余数与滑入比特 (固定锚定在 ANCHOR_COL + 2 工位)
            Elements row5_elements;
            int rem_lead = ANCHOR_COL + 2;
            if (rem_lead > 0) {
                row5_elements.push_back(text(std::string(rem_lead, ' ')));
            }
            std::string rem_bits;
            for (size_t r = 1; r < snap.remainder.size(); ++r) {
                rem_bits.push_back(snap.remainder[r]);
                rem_bits.push_back(' ');
            }
            if (!rem_bits.empty()) {
                rem_bits.pop_back();
            }
            row5_elements.push_back(text(rem_bits) | bold | color(Color::GreenLight));
            auto row5 = hbox({
                text(" 本步余数 ") | color(Color::GreenLight) | size(WIDTH, EQUAL, 12),
                separator(),
                hbox(std::move(row5_elements))
            });

            auto aligned_block = vbox({
                row_hex,
                row3,
                row_actual,
                row4,
                row4_5,
                row5
            }) | border | color(Color::BlueLight);

            Element summary_banner = (current_step + 1 == total_steps)
                ? hbox({ text(" ★ 模 2 除法全部完成! 最终余数 (FCS): ") | bold | color(Color::GreenLight),
                         text(snap.remainder.substr(1)) | bold | color(Color::Yellow),
                         text(" (0x" + utils::byte_to_hex(static_cast<uint8_t>(current_result.frame.fcs & 0xFF)) + ")") | bold | color(Color::Yellow) })
                : hbox({ text(" 提示: 红色/青色高亮比特表示当前计算窗口，数据流向左滑动; 点击 [下一步] 或 [播放] 连续演算") | dim });

            crc_detail = vbox({
                text("生成多项式 G(X): 1 0 0 0 0 0 1 1 1 (CRC-8 / 0x107)") | color(Color::Yellow) | bold,
                text("校验范围 (IEEE 802.3): 目的MAC(6B) + 源MAC(6B) + 长度(2B) + 载荷(" + 
                     std::to_string(current_result.frame.payload.size()) + "B) + 填充(" + 
                     std::to_string(current_result.frame.padding.size()) + "B) = " + 
                     std::to_string(crc_payload.size()) + "B (" + 
                     std::to_string(crc_payload.size() * 8) + " 比特 / 共 " + 
                     std::to_string(total_steps) + " 步)") | color(Color::CyanLight),
                separator(),
                macro_progress_bar,
                aligned_block,
                summary_banner
            });
        }

        auto crc_control_bar = hbox({
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