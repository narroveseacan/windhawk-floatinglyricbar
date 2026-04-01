// ==WindhawkMod==
// @id              floatinglyricbar
// @name            Floating Lyric Bar
// @description     A high-performance, native Windows media widget providing real-time, synchronized lyrics.
// @version         1.0.0
// @author          narrove
// @github          https://github.com/narroveseacan
// @include         explorer.exe
// @compilerOptions -lole32 -ldwmapi -lgdi32 -luser32 -lwindowsapp -lshcore -lgdiplus -lshell32 -lwinhttp
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Floating Lyric Bar (Barlyrics)
A high-performance, native Windows media widget providing real-time, synchronized karaoke lyrics directly on your taskbar or as a beautiful floating glass overlay.

### ✨ Major Features
* **🎤 Precision Karaoke Animation:** Experience smooth, 60fps syllable-by-syllable (TTML) gradient rendering. If syllable data is missing, it intelligently falls back to smooth line-level progress syncing.
* **🌍 Live Translation & Romanization:** Automatically translates foreign songs to your preferred language and generates Romanized text for Asian languages (Japanese, Korean, Mandarin, Thai, Russian etc.) using a custom Google Translate POST engine.
* **🔀 Multi-Provider Lyric Engine:** Aggregates lyrics from massive databases. It dynamically prioritizes Apple Music (via BetterLyrics API) for syllable sync, falling back to LRCLib and Legato (Kugou) for unparalleled coverage.
* **🖥️ Dual Display Modes:** * **Docked Mode:** Snaps perfectly into your Windows Taskbar alongside your tray icons.
    * **Floating Mode:** Detaches into a draggable desktop widget. Remembers its exact screen position.
    * **Locked Glass Mode:** Turns the floating widget into a 100% click-through, borderless, transparent overlay with beautiful text drop-shadows (perfect for pinning over games or wallpapers).
* **🎨 Native Windows 11 Styling:** Utilizes Desktop Window Manager (DWM) for authentic Acrylic blur, true transparent gradients, and smooth rounded corners.
* **🧠 Smart Window Management:** Automatically hides itself when you launch Fullscreen games or hit F11 in your browser. Includes an optional "Idle Timeout" to fade away when music is paused.
* **🎵 Universal Media Control:** Play, pause, skip, or scroll over the widget to adjust your volume. Works with Spotify, Apple Music, and natively prioritizes YouTube Music browser tabs.

  ### 📊 Provider Support
  Compared vs. original Better Lyrics extension and HotLyrics 热词:

  | # | Provider & Sync Level | Floating Lyric Bar | Current Implementation Details |
  |---|-----------------------|----------------------|--------------------------------|
  | 1 | **Better Lyrics (Syllable)** | ⚠️ Yes | Fetches TTML from `boidu.dev`, parses `<span>` word-timing tags. Falls back to `lrc` field in same response if no TTML. |
  | 2 | **Musixmatch (Word)** | ❌ No | Requires Turnstile/JWT (Browser-only). |
  | 3 | **YouTube Captions (Line)** | ❌ No | Not feasible in native C++ without browser context. |
  | 4 | **Better Lyrics (Line)** | ✅ Yes | Uses `lrc` field from the same `boidu.dev` response — no extra request needed. |
  | 5 | **LRCLib (Line)** | ✅ Yes | Core fallback for songs not in Apple Music catalog (e.g. indie/local artists). |
  | 6 | **NetEase (Line)** | ✅ Yes | Direct fetch from `music.163.com` API. Good coverage for Asian artists. |
  | 7 | **Better Lyrics Legato (Line)** | ✅ Yes | Fetches from `boidu.dev/kugou` (Kugou Music). Last-resort fallback. |
  | 8 | **Musixmatch (Line)** | ❌ No | Requires Turnstile/JWT (Browser-only). |
  | 9 | **YouTube Lyrics (Unsynced)** | ❌ No | Not feasible in native C++ without browser context. |
  | 10 | **LRCLib (Unsynced)** | ✅ Yes | Automatic fallback when no synced lyrics exist anywhere. |
  | 11 | **QQ Music (Line)** | ❌ No | Complex proprietary encryption (present in HotLyric). |

### ⌨️ Global Hotkeys
Take control of the widget from anywhere on your PC:
* `Ctrl + Shift + Alt + D` : **Snap to Taskbar** (Docked Mode)
* `Ctrl + Shift + Alt + F` : **Detach to Desktop** (Floating Mode - restores last known position)
* `Ctrl + Shift + Alt + S` : **Toggle Click-Through Lock** (Only works in Floating Mode)

### 🏆 Credits & Acknowledgments
This project stands on the shoulders of giants. Massive thanks to:
* **Taskbar Music Lounge:** Built upon the brilliant foundational WinRT/GSMTC architecture and taskbar-hooking logic originally created by **Hashah2311**.
* **Better Lyrics:** Deeply inspired by the amazing browser extension. Special thanks for the `boidu.dev` API that powers the incredibly accurate TTML syllable-sync parsing. 
* **LRCLib:** The incredible open-source lyric database (`lrclib.net`) serving as the rock-solid backbone for LRC line-synced lyrics.
* **Google:** For the Translate API powering the real-time subtitle generation. 
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- PanelWidth: 480
  $name: Panel Width (px)
- PanelHeight: 50
  $name: Panel Height (px)
- FontSize: 18
  $name: Font Size
- OffsetX: 10
  $name: Taskbar Offset X
- OffsetY: 0
  $name: Taskbar Offset Y
- AutoTheme: true
  $name: Adaptive System Theme (Light/Dark Mode)
- TextColor: FFFFFF
  $name: Custom Text Color (Hex code, overrides Auto Theme)
- BgOpacity: 0
  $name: Background Blur Opacity (0-255, 0 = Pure Glass)
- ButtonScale: 1.0
  $name: Interface Scale Multiplier (1.0 = Default, 2.0 = 4K Displays)
- DisplayMode: 0
  $name: Default Startup Mode
  $options:
    - 0: Docked (Taskbar)
    - 1: Floating (Draggable Overlay)
- IsLocked: false
  $name: Enable Click-Through (Floating Mode Only)
- HideFullscreen: false
  $name: Auto-hide during Fullscreen Apps/Games
- IdleTimeout: 0
  $name: Auto-hide when paused (Seconds, 0 to disable)
- EnableTranslation: true
  $name: Enable Lyric Translation
- TranslationLang: id
  $name: Target Translation Language Code (e.g., en, id, ja)
- EnableRomanization: true
  $name: Enable Language Romanization
- SyllableOffset: 250
  $name: Syllable-Sync Offset (ms)
- LineOffset: 750
  $name: Line-Sync Offset (ms)
*/
// ==/WindhawkModSettings==

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <dwmapi.h>
#include <gdiplus.h>
#include <iomanip>
#include <mutex>
#include <shcore.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>
#include <winhttp.h>
#include <locale.h>

// Global Locale for American formatting (Decimal point instead of comma)
_locale_t g_cLocale = _create_locale(LC_NUMERIC, "C");

// WinRT
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace Gdiplus;
using namespace std;
using namespace winrt;
using namespace Windows::Media::Control;
using namespace Windows::Storage::Streams;
using namespace Windows::Data::Json;

// --- Constants ---
const WCHAR *FONT_NAME = L"Segoe UI Variable Display";
const WCHAR *LRCLIB_API_URL = L"lrclib.net";
const WCHAR *BLYRICS_API_HOST = L"lyrics-api.boidu.dev";
const int HTTPS_PORT = INTERNET_DEFAULT_HTTPS_PORT;

// --- DWM API ---
typedef enum _WINDOWCOMPOSITIONATTRIB {
  WCA_ACCENT_POLICY = 19
} WINDOWCOMPOSITIONATTRIB;
typedef enum _ACCENT_STATE {
  ACCENT_DISABLED = 0,
  ACCENT_ENABLE_BLURBEHIND = 3,
  ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
  ACCENT_INVALID_STATE = 5
} ACCENT_STATE;
typedef struct _ACCENT_POLICY {
  ACCENT_STATE AccentState;
  DWORD AccentFlags;
  DWORD GradientColor;
  DWORD AnimationId;
} ACCENT_POLICY;
typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
  WINDOWCOMPOSITIONATTRIB Attribute;
  PVOID Data;
  SIZE_T SizeOfData;
} WINDOWCOMPOSITIONATTRIBDATA;
typedef BOOL(WINAPI *pSetWindowCompositionAttribute)(
    HWND, WINDOWCOMPOSITIONATTRIBDATA *);

// --- Z-Band API ---
enum ZBID {
  ZBID_DEFAULT = 0,
  ZBID_DESKTOP = 1,
  ZBID_UIACCESS = 2,
  ZBID_IMMERSIVE_IHM = 3,
  ZBID_IMMERSIVE_NOTIFICATION = 4,
  ZBID_IMMERSIVE_APPCHROME = 5,
  ZBID_IMMERSIVE_MOGO = 6,
  ZBID_IMMERSIVE_EDGY = 7,
  ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
  ZBID_IMMERSIVE_INACTIVEDOCK = 9,
  ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
  ZBID_IMMERSIVE_ACTIVEDOCK = 11,
  ZBID_IMMERSIVE_BACKGROUND = 12,
  ZBID_IMMERSIVE_SEARCH = 13,
  ZBID_GENUINE_WINDOWS = 14,
  ZBID_IMMERSIVE_RESTRICTED = 15,
  ZBID_SYSTEM_TOOLS = 16,
  ZBID_LOCK = 17,
  ZBID_ABOVELOCK_UX = 18,
};

typedef HWND(WINAPI *pCreateWindowInBand)(DWORD dwExStyle, LPCWSTR lpClassName,
                                          LPCWSTR lpWindowName, DWORD dwStyle,
                                          int x, int y, int nWidth, int nHeight,
                                          HWND hWndParent, HMENU hMenu,
                                          HINSTANCE hInstance, LPVOID lpParam,
                                          DWORD dwBand);

// --- Configurable State ---
struct ModSettings {
  int width = 480;
  int height = 50;
  int fontSize = 18;
  double buttonScale = 1.0;
  bool hideFullscreen = false;
  int idleTimeout = 0;
  int offsetX = 10;
  int offsetY = 0;
  bool autoTheme = true;
  DWORD manualTextColor = 0xFFFFFFFF;
  int bgOpacity = 0;
  wstring translationLang = L"id";
  bool enableTranslation = true;
  bool enableRomanization = true;
  int displayMode = 0; // 0 = Docked, 1 = Floating
  bool isLocked = false;
  int syllableOffset = 250;
  int lineOffset = 750;
} g_Settings;

// --- Global State ---
HWND g_hMediaWindow = NULL;
bool g_Running = true;
int g_HoverState = 0;
int g_FloatX = -9999;
int g_FloatY = -9999;
HWINEVENTHOOK g_TaskbarHook = nullptr;
UINT g_TaskbarCreatedMsg = RegisterWindowMessage(L"TaskbarCreated");

// Idle Tracking
int g_IdleSecondsCounter = 0;
bool g_IsHiddenByIdle = false;

// Data Model
struct LyricWord {
  wstring text;
  long startTimeMs;
  long durationMs;
};

struct LyricLine {
  wstring text;
  wstring translation;
  wstring romanization;
  long startTimeMs;
  long durationMs;
  vector<LyricWord> words;
};

struct MediaState {
  wstring title = L"Waiting for media...";
  wstring artist = L"";
  wstring album = L"";
  long durationMs = 0;
  bool isPlaying = false;
  bool hasMedia = false;
  Bitmap *albumArt = nullptr;
  vector<LyricLine> lyrics;
  int currentLineIndex = -1;
  int lastLineIndex = -1;
  LyricLine prevLine;
  ULONGLONG transitionStartTime = 0;
  long currentTimeMs = 0;
  ULONGLONG lastUpdateTick = 0;
  ULONGLONG lastSongChangeTime = 0;
  ULONGLONG lastHoverOutTime = 0;
  bool hasSyllables = false;
  mutex lock;
} g_MediaState;

// Animation
int g_ScrollOffset = 0;
int g_LyricScrollOffset = 0;
int g_PrevLyricScrollOffset = 0;
int g_TextWidth = 0;
bool g_IsScrolling = false;
int g_ScrollWait = 60;

// --- Settings ---
void LoadSettings() {
  g_Settings.width = Wh_GetIntSetting(L"PanelWidth");
  g_Settings.height = Wh_GetIntSetting(L"PanelHeight");
  g_Settings.fontSize = Wh_GetIntSetting(L"FontSize");
  g_Settings.offsetX = Wh_GetIntSetting(L"OffsetX");
  g_Settings.offsetY = Wh_GetIntSetting(L"OffsetY");
  g_Settings.autoTheme = Wh_GetIntSetting(L"AutoTheme") != 0;

  PCWSTR scaleStr = Wh_GetStringSetting(L"ButtonScale");
  if (scaleStr) {
    g_Settings.buttonScale = _wtof(scaleStr);
    Wh_FreeStringSetting(scaleStr);
  } else {
    g_Settings.buttonScale = 1.0;
  }
  if (g_Settings.buttonScale < 0.5)
    g_Settings.buttonScale = 0.5;
  if (g_Settings.buttonScale > 4.0)
    g_Settings.buttonScale = 4.0;

  g_Settings.hideFullscreen = Wh_GetIntSetting(L"HideFullscreen") != 0;
  g_Settings.idleTimeout = Wh_GetIntSetting(L"IdleTimeout");

  PCWSTR textHex = Wh_GetStringSetting(L"TextColor");
  DWORD textRGB = 0xFFFFFF;
  if (textHex) {
    if (wcslen(textHex) > 0)
      textRGB = wcstoul(textHex, nullptr, 16);
    Wh_FreeStringSetting(textHex);
  }
  g_Settings.manualTextColor = 0xFF000000 | textRGB;

  g_Settings.bgOpacity = Wh_GetIntSetting(L"BgOpacity");
  if (g_Settings.bgOpacity < 0)
    g_Settings.bgOpacity = 0;
  if (g_Settings.bgOpacity > 255)
    g_Settings.bgOpacity = 255;

  PCWSTR transLang = Wh_GetStringSetting(L"TranslationLang");
  if (transLang) {
    g_Settings.translationLang = transLang;
    Wh_FreeStringSetting(transLang);
  } else {
    g_Settings.translationLang = L"en";
  }

  g_Settings.enableTranslation = Wh_GetIntSetting(L"EnableTranslation") != 0;
  g_Settings.enableRomanization = Wh_GetIntSetting(L"EnableRomanization") != 0;
  g_Settings.displayMode = Wh_GetIntSetting(L"DisplayMode");
  g_Settings.isLocked = Wh_GetIntSetting(L"IsLocked") != 0;
  g_Settings.syllableOffset = Wh_GetIntSetting(L"SyllableOffset");
  g_Settings.lineOffset = Wh_GetIntSetting(L"LineOffset");

  if (g_Settings.width < 100)
    g_Settings.width = 480;
  if (g_Settings.height < 24)
    g_Settings.height = 50;
}

// --- WinRT / GSMTC ---
GlobalSystemMediaTransportControlsSessionManager g_SessionManager = nullptr;

Bitmap *StreamToBitmap(IRandomAccessStreamWithContentType const &stream) {
  if (!stream)
    return nullptr;
  IStream *nativeStream = nullptr;
  if (SUCCEEDED(CreateStreamOverRandomAccessStream(
          reinterpret_cast<IUnknown *>(winrt::get_abi(stream)),
          IID_PPV_ARGS(&nativeStream)))) {
    Bitmap *bmp = Bitmap::FromStream(nativeStream);
    nativeStream->Release();
    if (bmp && bmp->GetLastStatus() == Ok)
      return bmp;
    delete bmp;
  }
  return nullptr;
}

// --- LRC Parser ---
struct LRCParser {
  static long ParseTime(wstring timeStr) {
    size_t colon = timeStr.find(L':');
    if (colon != wstring::npos) {
      wstring minStr = timeStr.substr(0, colon);
      wstring secStr = timeStr.substr(colon + 1);
      float m = (float)_wtof_l(minStr.c_str(), g_cLocale);
      float s = (float)_wtof_l(secStr.c_str(), g_cLocale);
      return (long)((m * 60.0f + s) * 1000.0f);
    }
    return 0;
  }

  static vector<LyricLine> Parse(const wstring &lrc, long songDuration) {
    vector<LyricLine> lines;
    wstringstream ss(lrc);
    wstring line;
    while (getline(ss, line)) {
      if (line.empty())
        continue;

      size_t lastEnd = 0;
      vector<long> timestamps;
      while (true) {
        size_t start = line.find(L'[', lastEnd);
        size_t end = line.find(L']', start);
        if (start != wstring::npos && end != wstring::npos && end > start + 1) {
          wstring timePart = line.substr(start + 1, end - start - 1);
          timestamps.push_back(ParseTime(timePart));
          lastEnd = end + 1;
        } else
          break;
      }

      if (timestamps.empty())
        continue;
      wstring textPart = line.substr(lastEnd);

      for (long startTime : timestamps) {
        LyricLine l;
        l.startTimeMs = startTime;

        size_t wStart = 0;
        while ((wStart = textPart.find(L'<', wStart)) != wstring::npos) {
          size_t wEnd = textPart.find(L'>', wStart);
          if (wEnd != wstring::npos) {
            LyricWord w;
            w.startTimeMs =
                ParseTime(textPart.substr(wStart + 1, wEnd - wStart - 1));

            size_t nextW = textPart.find(L'<', wEnd);
            w.text = textPart.substr(wEnd + 1, nextW - wEnd - 1);
            l.words.push_back(w);
            wStart = wEnd + 1;
          } else
            break;
        }

        if (l.words.empty()) {
          l.text = textPart;
        } else {
          for (auto &w : l.words)
            l.text += w.text;
        }
        lines.push_back(l);
      }
    }

    sort(lines.begin(), lines.end(),
         [](const LyricLine &a, const LyricLine &b) {
           return a.startTimeMs < b.startTimeMs;
         });

    // Calculate durations (Precision Engine)
    for (size_t i = 0; i < lines.size(); i++) {
      if (i + 1 < lines.size()) {
        long gap = lines[i + 1].startTimeMs - lines[i].startTimeMs;
        lines[i].durationMs = (gap < 8000) ? gap : 5000; // Limit hang time
      } else {
        lines[i].durationMs = 5000;
      }

      if (!lines[i].words.empty()) {
        for (size_t j = 0; j < lines[i].words.size(); j++) {
          if (j + 1 < lines[i].words.size()) {
            lines[i].words[j].durationMs = lines[i].words[j + 1].startTimeMs -
                                           lines[i].words[j].startTimeMs;
          } else {
            lines[i].words[j].durationMs =
                lines[i].durationMs -
                (lines[i].words[j].startTimeMs - lines[i].startTimeMs);
          }
        }
      }
    }
    Wh_Log(L"Parsed %d lyric lines", (int)lines.size());
    return lines;
  }
};

// --- WinHTTP Fetcher ---
wstring HttpGet(const WCHAR *host, int port, const wstring &path) {
  HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return L"";

  HINTERNET hConnect = WinHttpConnect(hSession, host, port, 0);
  if (!hConnect) { WinHttpCloseHandle(hSession); return L""; }

  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
  if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return L""; }

  WinHttpAddRequestHeaders(hRequest, L"Lrclib-Client: WindhawkLyricbar/1.0\r\n", -1, WINHTTP_ADDREQ_FLAG_ADD);

  wstring result;
  if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
    if (WinHttpReceiveResponse(hRequest, NULL)) {
      DWORD dwStatusCode = 0;
      DWORD dwSize = sizeof(dwStatusCode);
      WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);

      if (dwStatusCode == 200) {
        string rawResponse; // FIX: Accumulate raw bytes to prevent UTF-8 chunk splitting
        DWORD dwSizeAvail = 0;
        do {
          if (!WinHttpQueryDataAvailable(hRequest, &dwSizeAvail)) break;
          if (dwSizeAvail == 0) break;
          vector<char> buffer(dwSizeAvail);
          DWORD dwDownloaded = 0;
          if (WinHttpReadData(hRequest, buffer.data(), dwSizeAvail, &dwDownloaded)) {
            rawResponse.append(buffer.data(), dwDownloaded);
          }
        } while (dwSizeAvail > 0);

        if (!rawResponse.empty()) {
          int wlen = MultiByteToWideChar(CP_UTF8, 0, rawResponse.c_str(), (int)rawResponse.length(), NULL, 0);
          if (wlen > 0) {
            vector<wchar_t> wbuf(wlen);
            MultiByteToWideChar(CP_UTF8, 0, rawResponse.c_str(), (int)rawResponse.length(), wbuf.data(), wlen);
            result.assign(wbuf.data(), wlen);
          }
        }
      }
    }
  }
  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);
  return result;
}

// --- Specific Fetcher for NetEase Anti-Scraping ---
wstring HttpGetNetEase(const WCHAR *host, int port, const wstring &path) {
  HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return L"";

  HINTERNET hConnect = WinHttpConnect(hSession, host, port, 0);
  if (!hConnect) { WinHttpCloseHandle(hSession); return L""; }

  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
  if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return L""; }

  // Bypass NetEase 403 Restrictions using explicit Referer & Cookie
  wstring headers = L"Referer: https://music.163.com/\r\nCookie: appver=1.5.0.75771; os=pc;\r\n";
  WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

  wstring result;
  if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
    if (WinHttpReceiveResponse(hRequest, NULL)) {
      DWORD dwStatusCode = 0;
      DWORD dwSize = sizeof(dwStatusCode);
      WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);

      if (dwStatusCode == 200) {
        string rawResponse;
        DWORD dwSizeAvail = 0;
        do {
          if (!WinHttpQueryDataAvailable(hRequest, &dwSizeAvail)) break;
          if (dwSizeAvail == 0) break;
          vector<char> buffer(dwSizeAvail);
          DWORD dwDownloaded = 0;
          if (WinHttpReadData(hRequest, buffer.data(), dwSizeAvail, &dwDownloaded)) {
            rawResponse.append(buffer.data(), dwDownloaded);
          }
        } while (dwSizeAvail > 0);

        if (!rawResponse.empty()) {
          int wlen = MultiByteToWideChar(CP_UTF8, 0, rawResponse.c_str(), (int)rawResponse.length(), NULL, 0);
          if (wlen > 0) {
            vector<wchar_t> wbuf(wlen);
            MultiByteToWideChar(CP_UTF8, 0, rawResponse.c_str(), (int)rawResponse.length(), wbuf.data(), wlen);
            result.assign(wbuf.data(), wlen);
          }
        }
      }
    }
  }
  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);
  return result;
}


// --- POST Helper for Google Translate ---
string URLEncodeUTF8(const wstring& str) {
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0, NULL, NULL);
  std::string utf8_str(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), &utf8_str[0], size_needed, NULL, NULL);

  stringstream ss;
  for (unsigned char c : utf8_str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      ss << c;
    } else if (c == ' ') {
      ss << "+";
    } else {
      ss << "%" << hex << uppercase << setw(2) << setfill('0') << (int)c;
    }
  }
  return ss.str();
}

wstring HttpPost(const WCHAR *host, int port, const wstring &path, const string &body) {
  HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return L"";

  HINTERNET hConnect = WinHttpConnect(hSession, host, port, 0);
  if (!hConnect) { WinHttpCloseHandle(hSession); return L""; }

  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
  if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return L""; }

  wstring headers = L"Content-Type: application/x-www-form-urlencoded\r\n";

  wstring result;
  if (WinHttpSendRequest(hRequest, headers.c_str(), (DWORD)-1, (LPVOID)body.data(), (DWORD)body.length(), (DWORD)body.length(), 0)) {
    if (WinHttpReceiveResponse(hRequest, NULL)) {
      DWORD dwStatusCode = 0;
      DWORD dwSize = sizeof(dwStatusCode);
      WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);

      if (dwStatusCode == 200) {
        string rawResponse; // FIX: Accumulate raw bytes to prevent UTF-8 chunk splitting
        DWORD dwSizeAvail = 0;
        do {
          if (!WinHttpQueryDataAvailable(hRequest, &dwSizeAvail)) break;
          if (dwSizeAvail == 0) break;
          vector<char> buffer(dwSizeAvail);
          DWORD dwDownloaded = 0;
          if (WinHttpReadData(hRequest, buffer.data(), dwSizeAvail, &dwDownloaded)) {
            rawResponse.append(buffer.data(), dwDownloaded);
          }
        } while (dwSizeAvail > 0);

        if (!rawResponse.empty()) {
          int wlen = MultiByteToWideChar(CP_UTF8, 0, rawResponse.c_str(), (int)rawResponse.length(), NULL, 0);
          if (wlen > 0) {
            vector<wchar_t> wbuf(wlen);
            MultiByteToWideChar(CP_UTF8, 0, rawResponse.c_str(), (int)rawResponse.length(), wbuf.data(), wlen);
            result.assign(wbuf.data(), wlen);
          }
        }
      }
    }
  }
  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);
  return result;
}

// --- Helpers ---
wstring URLEncode(wstring str) {
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, str.c_str(),
                                        (int)str.length(), NULL, 0, NULL, NULL);
  std::string utf8_str(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), &utf8_str[0],
                      size_needed, NULL, NULL);

  wstringstream ss;
  for (unsigned char c : utf8_str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      ss << (wchar_t)c;
    } else if (c == ' ') {
      ss << L"+";
    } else {
      ss << L"%" << hex << uppercase << setw(2) << setfill(L'0') << (int)c;
    }
  }
  return ss.str();
}

long ParseTimestamp(wstring ts) {
  if (ts.empty())
    return 0;

  if (ts.back() == L's') {
    return (long)(_wtof_l(ts.substr(0, ts.length() - 1).c_str(), g_cLocale) *
                  1000.0);
  }

  float a = 0, b = 0, c = 0;
  int count = _swscanf_l(ts.c_str(), L"%f:%f:%f", g_cLocale, &a, &b, &c);

  if (count == 3)
    return (long)((a * 3600.0f + b * 60.0f + c) * 1000.0f);
  if (count == 2)
    return (long)((a * 60.0f + b) * 1000.0f);
  if (count == 1)
    return (long)(a * 1000.0f);
  return 0;
}

vector<LyricLine> ParseTTML(wstring ttml, long songDuration) {
  // 1. Clean XML Namespaces & HTML Entities
  auto ReplaceAll = [](wstring &s, const wstring &from, const wstring &to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != wstring::npos) {
      s.replace(pos, from.length(), to);
      pos += to.length();
    }
  };

  // Strip Apple Music / iTunes prefixes so the parser can actually find the tags
  ReplaceAll(ttml, L"tt:span", L"span");
  ReplaceAll(ttml, L"itunes:span", L"span");
  ReplaceAll(ttml, L"am:span", L"span");
  ReplaceAll(ttml, L"tt:p", L"p");
  ReplaceAll(ttml, L"itunes:p", L"p");
  ReplaceAll(ttml, L"am:p", L"p");
  
  // Decode common XML entities
  ReplaceAll(ttml, L"&apos;", L"'");
  ReplaceAll(ttml, L"&quot;", L"\"");
  ReplaceAll(ttml, L"&amp;", L"&");
  ReplaceAll(ttml, L"&#39;", L"'");

  vector<LyricLine> lines;
  size_t pPos = 0;
  
  // 2. Relaxed Tag Searching (Removed the trailing space)
  while ((pPos = ttml.find(L"<p", pPos)) != wstring::npos) {
    // Match <p followed by space, newline, tab, or >
    if (pPos + 2 < ttml.length()) {
        wchar_t next = ttml[pPos + 2];
        if (next != L' ' && next != L'\t' && next != L'\n' && next != L'\r' && next != L'>') {
            pPos += 2;
            continue;
        }
    }
    size_t pEnd = ttml.find(L"</p>", pPos);
    if (pEnd == wstring::npos) break;

    LyricLine line;
    wstring pTag = ttml.substr(pPos, ttml.find(L">", pPos) - pPos);
    size_t beginAttr = pTag.find(L"begin=\"");
    size_t endAttr = pTag.find(L"end=\"");

    if (beginAttr != wstring::npos) {
      size_t valStart = beginAttr + 7;
      line.startTimeMs = ParseTimestamp(pTag.substr(valStart, pTag.find(L"\"", valStart) - valStart));
    }
    if (endAttr != wstring::npos) {
      size_t valStart = endAttr + 5;
      line.durationMs = ParseTimestamp(pTag.substr(valStart, pTag.find(L"\"", valStart) - valStart)) - line.startTimeMs;
    }

    wstring pContent = ttml.substr(ttml.find(L">", pPos) + 1, pEnd - (ttml.find(L">", pPos) + 1));
    size_t sPos = 0;

    // Relaxed span search to catch <span begin=... without failing on tabs/newlines
    while ((sPos = pContent.find(L"<span", sPos)) != wstring::npos) {
      size_t sEnd = pContent.find(L"</span>", sPos);
      if (sEnd == wstring::npos) break;

      wstring sTag = pContent.substr(sPos, pContent.find(L">", sPos) - sPos);
      LyricWord word;
      word.startTimeMs = line.startTimeMs;
      word.durationMs = 0;

      size_t sBegin = sTag.find(L"begin=\"");
      size_t sEndAttr = sTag.find(L"end=\"");

      // FIX A: Skip container spans that have no begin/end timing.
      // Apple Music TTML wraps word <span>s inside an outer <span> with no timing attrs.
      // Without this check, the outer span swallows the inner first word into a bogus entry.
      if (sBegin == wstring::npos) {
        sPos += 5; // advance past "<span" and keep searching
        continue;
      }

      if (sBegin != wstring::npos) {
        size_t vStart = sBegin + 7;
        word.startTimeMs = ParseTimestamp(sTag.substr(vStart, sTag.find(L"\"", vStart) - vStart));
      }
      if (sEndAttr != wstring::npos) {
        size_t vStart = sEndAttr + 5;
        word.durationMs = ParseTimestamp(sTag.substr(vStart, sTag.find(L"\"", vStart) - vStart)) - word.startTimeMs;
      }

      // FIX B: Extract ONLY the text inside <span>...</span>, nothing after </span>.
      // The old code appended inter-span content (newlines + indentation) to word.text,
      // causing line.text to contain \n characters, which made GDI+ MeasureString
      // return wrong multi-line dimensions and broke the karaoke gradient entirely.
      size_t contentStart = pContent.find(L">", sPos) + 1;
      word.text = pContent.substr(contentStart, sEnd - contentStart);

      // Strip any leaked tags (e.g. nested markup inside a span)
      size_t leakStart;
      while ((leakStart = word.text.find(L"<")) != wstring::npos) {
        size_t leakEnd = word.text.find(L">", leakStart);
        if (leakEnd != wstring::npos) {
          word.text.erase(leakStart, leakEnd - leakStart + 1);
        } else break;
      }

      // Skip words that are pure whitespace (empty placeholder spans)
      if (word.text.find_first_not_of(L" \t\r\n") == wstring::npos) {
        sPos = sEnd + 7;
        continue;
      }

      line.words.push_back(word);
      line.text += word.text;
      sPos = sEnd + 7;
    }

    if (line.text.empty() || line.text.find_first_not_of(L" \t\r\n") == wstring::npos) {
      line.text = pContent;
      size_t leakStart;
      while ((leakStart = line.text.find(L"<")) != wstring::npos) {
        size_t leakEnd = line.text.find(L">", leakStart);
        if (leakEnd != wstring::npos) line.text.erase(leakStart, leakEnd - leakStart + 1);
        else break;
      }
      if (line.text.empty() || line.text.find_first_not_of(L" \t\r\n") == wstring::npos) {
        line.text = L"𝅘𝅥𝅮 𝅘𝅥𝅮 𝅘𝅥𝅮";
      }
    }

    if (!line.words.empty()) {
      long lastWordEnd = line.words.back().startTimeMs + line.words.back().durationMs;
      line.durationMs = lastWordEnd - line.startTimeMs;
      if (line.durationMs < 0) line.durationMs = 0;
    }

    lines.push_back(line);
    pPos = pEnd + 4;
  }
  return lines;
}

bool FetchFromBetterLyrics(wstring title, wstring artist, wstring album,
                           long durationMs, vector<LyricLine> &outLyrics) {
  
  // Helper to parse the JSON response
  auto ParseResponse = [&](const wstring& j) -> bool {
    if (j.empty()) return false;
    try {
        JsonObject obj = JsonObject::Parse(j);

        // 1. Try TTML first (Apple Music syllable/word sync)
        if (obj.HasKey(L"ttml")) {
            try {
                wstring ttml = obj.GetNamedString(L"ttml").c_str();
                if (!ttml.empty()) {
                    Wh_Log(L"TTML received (first 120 chars): %.120s", ttml.c_str());
                    outLyrics = ParseTTML(ttml, durationMs);
                    if (!outLyrics.empty()) return true;
                    Wh_Log(L"TTML parse produced 0 lines - falling to lrc");
                }
            } catch (...) { Wh_Log(L"TTML field threw on access"); }
        }

        // 2. Fallback: use the lrc field from the same response
        // Avoids a second HTTP round-trip to LRCLib for the same song.
        if (obj.HasKey(L"lrc")) {
            try {
                wstring lrc = obj.GetNamedString(L"lrc").c_str();
                if (!lrc.empty()) {
                    Wh_Log(L"BLyrics: no TTML, using lrc fallback");
                    outLyrics = LRCParser::Parse(lrc, durationMs);
                    if (!outLyrics.empty()) return true;
                }
            } catch (...) {}
        }

    } catch (...) { Wh_Log(L"BLyrics JSON parse failed"); }
    return false;
  };

  // ATTEMPT 1: Strict Match (Includes duration with ±2s tolerance)
  long durSec = durationMs / 1000;
  for (long delta : {0L, 1L, -1L, 2L, -2L}) {
      wstring path = L"/getLyrics?s=" + URLEncode(title) + L"&a=" + URLEncode(artist) 
                   + L"&d=" + to_wstring(durSec + delta);
      if (!album.empty()) path += L"&al=" + URLEncode(album);
      if (ParseResponse(HttpGet(BLYRICS_API_HOST, HTTPS_PORT, path))) {
          Wh_Log(L"BLyrics strict match succeeded with delta=%lds", delta);
          return true;
      }
  }

  // ATTEMPT 2: Loose Match (Drop duration and album constraints)
  // YouTube Music durations often differ from Apple Music. Dropping it fixes most misses.
  Wh_Log(L"BLyrics strict match failed. Attempting loose match...");
  
  // Clean up the artist string (e.g., "Lady Gaga & Bruno Mars" -> "Lady Gaga")
  wstring looseArtist = artist;
  size_t ampPos = looseArtist.find(L"&");
  if (ampPos != wstring::npos) looseArtist = looseArtist.substr(0, ampPos);
  
  size_t commaPos = looseArtist.find(L",");
  if (commaPos != wstring::npos) looseArtist = looseArtist.substr(0, commaPos);
  
  // Trim trailing spaces
  size_t last = looseArtist.find_last_not_of(L" \t\r\n");
  if (last != wstring::npos) looseArtist = looseArtist.substr(0, last + 1);

  wstring loosePath = L"/getLyrics?s=" + URLEncode(title) + L"&a=" + URLEncode(looseArtist);
  
  return ParseResponse(HttpGet(BLYRICS_API_HOST, HTTPS_PORT, loosePath));
}

bool FetchFromLegato(wstring title, wstring artist, wstring album,
                     long durationMs, vector<LyricLine> &outLyrics) {
  wstring path = L"/kugou/getLyrics?s=" + URLEncode(title) + L"&a=" +
                 URLEncode(artist) + L"&d=" + to_wstring(durationMs / 1000);
  if (!album.empty())
    path += L"&al=" + URLEncode(album);

  wstring json = HttpGet(BLYRICS_API_HOST, HTTPS_PORT, path);
  if (json.empty())
    return false;

  try {
    JsonObject obj = JsonObject::Parse(json);
    if (obj.HasKey(L"lyrics")) {
      wstring lrc = obj.GetNamedString(L"lyrics").c_str();
      outLyrics = LRCParser::Parse(lrc, durationMs);
      return !outLyrics.empty();
    }
  } catch (...) {
  }
  return false;
}

bool FetchFromLRCLib(wstring title, wstring artist, wstring album,
                     long durationMs, vector<LyricLine> &outLyrics) {
  wstring path = L"/api/get?track_name=" + URLEncode(title) + L"&artist_name=" +
                 URLEncode(artist);
  if (!album.empty())
    path += L"&album_name=" + URLEncode(album);
  if (durationMs > 0)
    path += L"&duration=" + to_wstring(durationMs / 1000);

  wstring json = HttpGet(LRCLIB_API_URL, HTTPS_PORT, path);
  if (json.empty()) {
    wstring searchPath = L"/api/search?q=" + URLEncode(artist + L" " + title);
    json = HttpGet(LRCLIB_API_URL, HTTPS_PORT, searchPath);
    if (json.empty() || json.length() < 10)
      return false;
  }

  try {
    size_t syncedStart = json.find(L"\"syncedLyrics\":");
    if (syncedStart == wstring::npos)
      return false;
    size_t valueStart = json.find(L"\"", syncedStart + 15);
    if (valueStart == wstring::npos)
      return false;
    valueStart++;
    size_t valueEnd = valueStart;
    while ((valueEnd = json.find(L"\"", valueEnd)) != wstring::npos) {
      if (json[valueEnd - 1] != L'\\')
        break;
      valueEnd++;
    }
    if (valueEnd == wstring::npos)
      return false;

    wstring lrc = json.substr(valueStart, valueEnd - valueStart);
    auto ReplaceAll = [](wstring &s, const wstring &from, const wstring &to) {
      size_t pos = 0;
      while ((pos = s.find(from, pos)) != wstring::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
      }
    };
    ReplaceAll(lrc, L"\\n", L"\n");
    ReplaceAll(lrc, L"\\\"", L"\"");
    ReplaceAll(lrc, L"\\r", L"");

    outLyrics = LRCParser::Parse(lrc, durationMs);
    return !outLyrics.empty();
  } catch (...) {
  }
  return false;
}

bool FetchFromNetEase(wstring title, wstring artist, wstring album,
                      long durationMs, vector<LyricLine> &outLyrics) {
  wstring searchPath = L"/api/search/get/web?s=" + URLEncode(title + L" " + artist) +
                       L"&type=1&offset=0&total=true&limit=1";
  wstring searchJson = HttpGetNetEase(L"music.163.com", INTERNET_DEFAULT_HTTPS_PORT, searchPath);
  if (searchJson.empty())
    return false;

  try {
    JsonObject obj = JsonObject::Parse(searchJson);
    if (!obj.HasKey(L"result")) return false;
    JsonObject resultObj = obj.GetNamedObject(L"result");
    
    if (!resultObj.HasKey(L"songs")) return false;
    JsonArray songsArr = resultObj.GetNamedArray(L"songs");
    
    if (songsArr.Size() == 0) return false;
    JsonObject firstSong = songsArr.GetAt(0).GetObject();
    double songId = firstSong.GetNamedNumber(L"id");

    wstring lyricPath = L"/api/song/lyric?id=" + to_wstring((long long)songId) + L"&lv=1&kv=1&tv=-1";
    wstring lyricJson = HttpGetNetEase(L"music.163.com", INTERNET_DEFAULT_HTTPS_PORT, lyricPath);
    if (lyricJson.empty())
      return false;

    JsonObject lObj = JsonObject::Parse(lyricJson);
    if (lObj.HasKey(L"lrc")) {
      JsonObject lrcObj = lObj.GetNamedObject(L"lrc");
      wstring lrc = lrcObj.GetNamedString(L"lyric").c_str();
      
      auto ReplaceAll = [](wstring &s, const wstring &from, const wstring &to) {
        size_t pos = 0;
        while ((pos = s.find(from, pos)) != wstring::npos) {
          s.replace(pos, from.length(), to);
          pos += to.length();
        }
      };
      ReplaceAll(lrc, L"\\n", L"\n");
      ReplaceAll(lrc, L"\\\"", L"\"");
      ReplaceAll(lrc, L"\\r", L"");

      outLyrics = LRCParser::Parse(lrc, durationMs);
      
      // Clean up common Netease LRC artifacts
      for (auto& line : outLyrics) {
          if (line.text.size() > 4 && line.text[0] == L'[') {
              size_t br = line.text.find(L']');
              if (br != wstring::npos && br < 15) {
                  line.text.erase(0, br + 1);
              }
          }
      }

      return !outLyrics.empty();
    }
  } catch (...) {
  }
  return false;
}

// --- Metadata Scrubber for Stricter APIs (Apple Music) ---
wstring CleanMediaString(wstring str) {
  // 1. Aggressively strip anything inside parentheses () or brackets []
  size_t pOpen = str.find(L'(');
  if (pOpen != wstring::npos) str = str.substr(0, pOpen);

  size_t bOpen = str.find(L'[');
  if (bOpen != wstring::npos) str = str.substr(0, bOpen);

  // 2. Normalize and strip common trailer tags (feat, remastered, single etc)
  wstring lowerStr = str;
  transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);

  const wchar_t* tags[] = {
      L" feat.", L" ft.", L" - remastered", L" (remastered", L" - single", 
      L" (single", L" (official", L" [official", L" - radio edit", L" - deluxe", L" - bonus"
  };

  for (const wchar_t* tag : tags) {
      size_t pos = lowerStr.find(tag);
      if (pos != wstring::npos) {
          str = str.substr(0, pos);
          lowerStr = lowerStr.substr(0, pos);
      }
  }

  // 3. Trim trailing spaces
  size_t last = str.find_last_not_of(L" \t\r\n");
  if (last != wstring::npos) {
      str = str.substr(0, last + 1);
  }
  return str;
}

void FetchLyrics(wstring title, wstring artist, wstring album,
                 long durationMs) {
  vector<LyricLine> lines;
  bool found = false;

  // Scrub the titles for strict API compatibility
  wstring cleanTitle = CleanMediaString(title);
  wstring cleanAlbum = CleanMediaString(album);

  Wh_Log(L"Fetching lyrics for: %s (Cleaned: %s) - %s", title.c_str(), cleanTitle.c_str(), artist.c_str());

  // Pass the CLEANED titles into the fetchers!
  if (FetchFromBetterLyrics(cleanTitle, artist, cleanAlbum, durationMs, lines)) {
    bool hasSyl = false;
    for (auto &l : lines) if (!l.words.empty()) { hasSyl = true; break; }
    if (hasSyl) {
        Wh_Log(L"Lyrics found from BLyrics (Syllable Sync)");
    } else {
        Wh_Log(L"Lyrics found from BLyrics (Line Sync only - no span data)");
    }
    found = true;
  } else if (FetchFromLRCLib(cleanTitle, artist, cleanAlbum, durationMs, lines)) {
    Wh_Log(L"Lyrics found from LRCLib");
    found = true;
  } else if (FetchFromNetEase(cleanTitle, artist, cleanAlbum, durationMs, lines)) {
    Wh_Log(L"Lyrics found from NetEase");
    found = true;
  } else if (FetchFromLegato(cleanTitle, artist, cleanAlbum, durationMs, lines)) {
    Wh_Log(L"Lyrics found from Legato");
    found = true;
  }

  if (!found) {
    Wh_Log(L"No lyrics found for %s", cleanTitle.c_str());
    return;
  }

  // Fetch Translations in batch if needed
  if (g_Settings.enableTranslation && !g_Settings.translationLang.empty() &&
      !lines.empty()) {
    bool needsTranslation = false;
    for (auto &l : lines) {
      if (l.translation.empty()) {
        needsTranslation = true;
        break;
      }
    }

    if (needsTranslation) {
      wstring allText;
      for (auto &l : lines)
        allText += l.text + L"\n\n";

      // FIX: Use HttpPost so the URL isn't truncated
      wstring transPath = L"/translate_a/single?client=gtx&sl=auto&tl=" + g_Settings.translationLang + L"&dt=t";
      string transBody = "q=" + URLEncodeUTF8(allText);
      wstring transJson = HttpPost(L"translate.googleapis.com", INTERNET_DEFAULT_HTTPS_PORT, transPath, transBody);

      if (!transJson.empty()) {
        try {
          JsonArray root = JsonArray::Parse(transJson);
          JsonArray blocks = root.GetAt(0).GetArray();
          wstring fullTranslation = L"";

          // Accurately extract only the translated text blocks
          for (uint32_t i = 0; i < blocks.Size(); i++) {
            JsonArray block = blocks.GetAt(i).GetArray();
            if (block.Size() > 0 &&
                block.GetAt(0).ValueType() == JsonValueType::String) {
              fullTranslation += block.GetAt(0).GetString().c_str();
            }
          }

          wstringstream ss(fullTranslation);
          wstring tLine;
          int lineIdx = 0;
          while (getline(ss, tLine) && lineIdx < (int)lines.size()) {
            if (!tLine.empty() && tLine.back() == L'\r')
              tLine.pop_back();
            if (tLine.empty()) continue; // Skip the extra blank lines we injected
            if (lines[lineIdx].translation.empty()) {
              lines[lineIdx].translation = tLine;
            }
            lineIdx++;
          }
        } catch (...) {
          Wh_Log(L"Failed to parse Translation JSON");
        }
      }
    }
  }

  // Fetch Romanization if needed
  if (g_Settings.enableRomanization && !lines.empty()) {
    bool needsRomanization = false;
    for (auto &l : lines) {
      if (l.romanization.empty()) {
        needsRomanization = true;
        break;
      }
    }

    if (needsRomanization) {
      wstring allText;
      // FIX: Use a strong punctuation delimiter (|||) because Google's Japanese phonetic engine strips newlines
      for (size_t i = 0; i < lines.size(); i++) {
        allText += lines[i].text;
        if (i < lines.size() - 1) allText += L" ||| "; 
      }

      wstring romPath = L"/translate_a/single?client=gtx&sl=auto&tl=en-Latn&dt=rm";
      string romBody = "q=" + URLEncodeUTF8(allText);
      wstring romJson = HttpPost(L"translate.googleapis.com", INTERNET_DEFAULT_HTTPS_PORT, romPath, romBody);

      if (!romJson.empty()) {
        try {
          JsonArray root = JsonArray::Parse(romJson);
          JsonArray blocks = root.GetAt(0).GetArray();
          wstring fullRom = L"";

          for (uint32_t i = 0; i < blocks.Size(); i++) {
            JsonArray block = blocks.GetAt(i).GetArray();
            if (block.Size() > 2 && block.GetAt(2).ValueType() == JsonValueType::String) {
              fullRom += block.GetAt(2).GetString().c_str();
            } else if (block.Size() > 3 && block.GetAt(3).ValueType() == JsonValueType::String) {
              fullRom += block.GetAt(3).GetString().c_str();
            }
          }

          // --- NEW FIX: Normalize Google's Altered Pipes ---
          auto ReplaceAll = [](wstring &s, const wstring &from, const wstring &to) {
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != wstring::npos) {
              s.replace(pos, from.length(), to);
              pos += to.length();
            }
          };
          
          // Force Full-width Japanese pipes and spaced pipes back into standard ASCII
          ReplaceAll(fullRom, L"｜", L"|"); // Convert Full-width to standard
          ReplaceAll(fullRom, L"| ", L"|"); // Crush trailing spaces
          ReplaceAll(fullRom, L" |", L"|"); // Crush leading spaces

          // FIX: Slice the Romanized string back into lines using our delimiter
          size_t start = 0;
          size_t end = 0;
          int lineIdx = 0;
          while ((end = fullRom.find(L"|||", start)) != wstring::npos && lineIdx < (int)lines.size()) {
            wstring rLine = fullRom.substr(start, end - start);
            
            // Trim surrounding spaces
            size_t first = rLine.find_first_not_of(L" \t\r\n");
            if (first != wstring::npos) {
                rLine = rLine.substr(first, rLine.find_last_not_of(L" \t\r\n") - first + 1);
                if (lines[lineIdx].romanization.empty()) lines[lineIdx].romanization = rLine;
            }
            start = end + 3; // Skip the "|||"
            lineIdx++;
          }
          
          // Capture the very last line
          if (lineIdx < (int)lines.size() && start < fullRom.length()) {
            wstring rLine = fullRom.substr(start);
            size_t first = rLine.find_first_not_of(L" \t\r\n");
            if (first != wstring::npos) {
                rLine = rLine.substr(first, rLine.find_last_not_of(L" \t\r\n") - first + 1);
                if (lines[lineIdx].romanization.empty()) lines[lineIdx].romanization = rLine;
            }
          }
        } catch (...) {
          Wh_Log(L"Failed to parse Romanization JSON");
        }
      }
    }
  }

  // Check if any line contains syllable/word data
  bool hasSyl = false;
  for (auto &l : lines) {
    if (!l.words.empty()) {
      hasSyl = true;
      break;
    }
  }

  lock_guard<mutex> guard(g_MediaState.lock);
  g_MediaState.lyrics = lines;
  g_MediaState.hasSyllables = hasSyl;
}

void UpdateMediaInfo() {
  try {
    if (!g_SessionManager) {
      g_SessionManager =
          GlobalSystemMediaTransportControlsSessionManager::RequestAsync()
              .get();
    }
    if (!g_SessionManager)
      return;

    auto sessionsList = g_SessionManager.GetSessions();
    GlobalSystemMediaTransportControlsSession session = nullptr;

    // Priority for YouTube Music if available
    for (auto const &s : sessionsList) {
      try {
        auto props = s.TryGetMediaPropertiesAsync().get();
        wstring title = props.Title().c_str();
        if (title.find(L"YouTube Music") != wstring::npos) {
          session = s;
          break;
        }
      } catch (...) {
      }
    }

    if (!session) {
      for (auto const &s : sessionsList) {
        auto pb = s.GetPlaybackInfo();
        if (pb && pb.PlaybackStatus() ==
                      GlobalSystemMediaTransportControlsSessionPlaybackStatus::
                          Playing) {
          session = s;
          break;
        }
      }
    }

    // Retain last media info if session is lost
    if (!session) {
      lock_guard<mutex> guard(g_MediaState.lock);
      if (g_MediaState.isPlaying) {
        Wh_Log(L"Media Session Inactive/Lost");
        g_MediaState.isPlaying = false;
      }
      return;
    }

    if (session) {
      auto props = session.TryGetMediaPropertiesAsync().get();
      auto info = session.GetPlaybackInfo();
      auto timeline = session.GetTimelineProperties();

      lock_guard<mutex> guard(g_MediaState.lock);

      wstring newTitle = props.Title().c_str();
      wstring newArtist = props.Artist().c_str();
      wstring newAlbum = props.AlbumTitle().c_str();
      long newDuration = (long)timeline.EndTime().count() / 10000;

      bool songChanged = (newTitle != g_MediaState.title || newArtist != g_MediaState.artist);
      if (songChanged) {
        Wh_Log(L"New Song Detected: %ls - %ls", newTitle.c_str(),
               newArtist.c_str());
        g_MediaState.lastSongChangeTime = GetTickCount64();
        g_ScrollOffset = 0; // Reset scroll for new song
        g_ScrollWait = 60;

        if (g_MediaState.albumArt) {
          delete g_MediaState.albumArt;
          g_MediaState.albumArt = nullptr;
        }

        g_MediaState.lyrics.clear();
        g_MediaState.currentLineIndex = -1;

        // Spawn fetch thread only once per song
        thread([newTitle, newArtist, newAlbum, newDuration]() {
          FetchLyrics(newTitle, newArtist, newAlbum, newDuration);
        }).detach();
      }

      // Try to fetch art if missing (fixes delayed thumbnail issue)
      if (!g_MediaState.albumArt) {
        auto thumbRef = props.Thumbnail();
        if (thumbRef) {
          try {
            auto stream = thumbRef.OpenReadAsync().get();
            g_MediaState.albumArt = StreamToBitmap(stream);
          } catch (...) {}
        }
      }

      g_MediaState.title = newTitle;
      g_MediaState.artist = newArtist;
      g_MediaState.album = newAlbum;
      g_MediaState.durationMs = newDuration;
      // Sync lyrics index
      long currentTime = (long)(timeline.Position().count() / 10000);
      if (info.PlaybackStatus() ==
          GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing) {
        auto lastUpdated = timeline.LastUpdatedTime();
        auto now = winrt::clock::now();
        auto diff = now - lastUpdated;
        currentTime += (long)(diff.count() / 10000);

        // Dynamic Offset from Settings: (Default 250ms Syllable / 750ms Line)
        currentTime +=
            g_MediaState.hasSyllables ? g_Settings.syllableOffset : g_Settings.lineOffset;
      }
      g_MediaState.currentTimeMs = currentTime;
      g_MediaState.lastUpdateTick = GetTickCount64();

      bool wasPlaying = g_MediaState.isPlaying;
      g_MediaState.isPlaying =
          (info.PlaybackStatus() ==
           GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing);
      if (g_MediaState.isPlaying && !wasPlaying) {
        g_MediaState.lastSongChangeTime =
            GetTickCount64(); // Trigger 1s on resume
      }
      g_MediaState.hasMedia = true;
    }
  } catch (...) {
    lock_guard<mutex> guard(g_MediaState.lock);
    g_MediaState.isPlaying = false;
  }
}

void SendMediaCommand(int cmd) {
  try {
    if (!g_SessionManager)
      return;
    auto session = g_SessionManager.GetCurrentSession();
    if (session) {
      if (cmd == 1)
        session.TrySkipPreviousAsync();
      else if (cmd == 2)
        session.TryTogglePlayPauseAsync();
      else if (cmd == 3)
        session.TrySkipNextAsync();
    }
  } catch (...) {
  }
}

// --- Visuals ---
bool IsSystemLightMode() {
  DWORD value = 0;
  DWORD size = sizeof(value);
  if (RegGetValueW(
          HKEY_CURRENT_USER,
          L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
          L"SystemUsesLightTheme", RRF_RT_DWORD, nullptr, &value,
          &size) == ERROR_SUCCESS) {
    return value != 0;
  }
  return false;
}

DWORD GetCurrentTextColor() {
  if (g_Settings.autoTheme)
    return IsSystemLightMode() ? 0xFF000000 : 0xFFFFFFFF;
  return g_Settings.manualTextColor;
}

void UpdateAppearance(HWND hwnd) {
  bool isLockedFloating = (g_Settings.displayMode == 1 && g_Settings.isLocked);

  // FIX: Disable rounded corners in locked mode.
  // This completely removes the Windows 11 1px border and system box shadow.
  DWM_WINDOW_CORNER_PREFERENCE preference = isLockedFloating ? DWMWCP_DONOTROUND : DWMWCP_ROUND;
  DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

  LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

  if (isLockedFloating) {
    exStyle |= WS_EX_TRANSPARENT; // Ignore mouse clicks
  } else {
    exStyle &= ~WS_EX_TRANSPARENT; // Accept mouse clicks
  }
  SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

  HMODULE hUser = GetModuleHandle(L"user32.dll");
  if (hUser) {
    auto SetComp = (pSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
    if (SetComp) {
      // Use ACCENT_ENABLE_TRANSPARENTGRADIENT (2) to force true per-pixel alpha transparency
      ACCENT_POLICY policy = { (ACCENT_STATE)2, 2, 0x00000000, 0 };

      if (!isLockedFloating) {
          DWORD tint = 0;
          if (g_Settings.autoTheme) {
            tint = IsSystemLightMode() ? 0x40FFFFFF : 0x40000000;
          } else {
            tint = (g_Settings.bgOpacity << 24) | (0xFFFFFF);
          }
          policy = {ACCENT_ENABLE_ACRYLICBLURBEHIND, 0, tint, 0};
      }

      WINDOWCOMPOSITIONATTRIBDATA data = {WCA_ACCENT_POLICY, &policy, sizeof(ACCENT_POLICY)};
      SetComp(hwnd, &data);
    }
  }
}

void AddRoundedRect(GraphicsPath &path, int x, int y, int w, int h, int r) {
  int d = r * 2;
  path.AddArc(x, y, d, d, 180, 90);
  path.AddArc(x + w - d, y, d, d, 270, 90);
  path.AddArc(x + w - d, y + h - d, d, d, 0, 90);
  path.AddArc(x, y + h - d, d, d, 90, 90);
  path.CloseFigure();
}

void DrawMediaPanel(HDC hdc, int width, int height) {
  Graphics graphics(hdc);
  graphics.SetSmoothingMode(SmoothingModeAntiAlias);
  graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
  graphics.Clear(Color(0, 0, 0, 0));

  bool isLockedFloating = (g_Settings.displayMode == 1 && g_Settings.isLocked);
  Color mainColor{isLockedFloating ? 0xFFFFFFFF : GetCurrentTextColor()};
  MediaState state;
  {
    lock_guard<mutex> guard(g_MediaState.lock);
    state.title = g_MediaState.title;
    state.artist = g_MediaState.artist;
    state.albumArt =
        g_MediaState.albumArt ? g_MediaState.albumArt->Clone() : nullptr;
    state.hasMedia = g_MediaState.hasMedia;
    state.isPlaying = g_MediaState.isPlaying;
    state.lyrics = g_MediaState.lyrics;
    state.currentLineIndex = g_MediaState.currentLineIndex;
    state.currentTimeMs = g_MediaState.currentTimeMs;
    state.lastUpdateTick = g_MediaState.lastUpdateTick;
    state.transitionStartTime = g_MediaState.transitionStartTime;
    state.prevLine = g_MediaState.prevLine;
    state.hasSyllables = g_MediaState.hasSyllables;
  }

  // 1. Album Art (Rounded)
  int artSize = height - 12;
  int artX = 6;
  int artY = 6;
  int textX = 6;

  if (!isLockedFloating) {
    GraphicsPath path;
    AddRoundedRect(path, artX, artY, artSize, artSize, 8);

    if (state.albumArt) {
      graphics.SetClip(&path);
      graphics.DrawImage(state.albumArt, artX, artY, artSize, artSize);
      graphics.ResetClip();
      delete state.albumArt;
    } else {
      SolidBrush placeBrush{Color(40, 128, 128, 128)};
      graphics.FillPath(&placeBrush, &path);
    }
    textX = artX + artSize + (int)(12 * g_Settings.buttonScale);
  }

  // 2. Controls (Scaled)
  double scale = g_Settings.buttonScale;
  int startControlX = textX;
  int controlY = height / 2;

  SolidBrush iconBrush{mainColor};
  SolidBrush hoverBrush{Color(255, mainColor.GetRed(), mainColor.GetGreen(),
                              mainColor.GetBlue())};
  SolidBrush activeBg{
      Color(40, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue())};

  float circleR = 12.0f * (float)scale;
  float iconW = 8.0f * (float)scale;
  float iconH = 12.0f * (float)scale;
  float gap = 28.0f * (float)scale;

  if (!isLockedFloating) {
    // Prev
    float pX = (float)startControlX;
    if (g_HoverState == 1)
      graphics.FillEllipse(&activeBg, pX - circleR, (float)controlY - circleR,
                           circleR * 2, circleR * 2);
    PointF prevPts[3] = {PointF(pX + iconW, (float)controlY - (iconH / 2)),
                         PointF(pX + iconW, (float)controlY + (iconH / 2)),
                         PointF(pX, (float)controlY)};
    graphics.FillPolygon(g_HoverState == 1 ? &hoverBrush : &iconBrush, prevPts,
                         3);
    graphics.FillRectangle(g_HoverState == 1 ? &hoverBrush : &iconBrush, pX,
                           (float)controlY - (iconH / 2), 2.0f * (float)scale,
                           iconH);

    // Play/Pause
    float plX = pX + gap;
    if (g_HoverState == 2)
      graphics.FillEllipse(&activeBg, plX - circleR, (float)controlY - circleR,
                           circleR * 2, circleR * 2);
    if (state.isPlaying) {
      float barW = 3.0f * (float)scale;
      float barH = 14.0f * (float)scale;
      graphics.FillRectangle(g_HoverState == 2 ? &hoverBrush : &iconBrush,
                             plX - (barW + 1), (float)controlY - (barH / 2),
                             barW, barH);
      graphics.FillRectangle(g_HoverState == 2 ? &hoverBrush : &iconBrush,
                             plX + 1, (float)controlY - (barH / 2), barW, barH);
    } else {
      float playW = 10.0f * (float)scale;
      float playH = 16.0f * (float)scale;
      PointF playPts[3] = {
          PointF(plX - (playW / 2), (float)controlY - (playH / 2)),
          PointF(plX - (playW / 2), (float)controlY + (playH / 2)),
          PointF(plX + (playW / 2), (float)controlY)};
      graphics.FillPolygon(g_HoverState == 2 ? &hoverBrush : &iconBrush, playPts,
                           3);
    }

    // Next
    float nX = plX + gap;
    if (g_HoverState == 3)
      graphics.FillEllipse(&activeBg, nX - circleR, (float)controlY - circleR,
                           circleR * 2, circleR * 2);
    PointF nextPts[3] = {PointF(nX - iconW, (float)controlY - (iconH / 2)),
                         PointF(nX - iconW, (float)controlY + (iconH / 2)),
                         PointF(nX, (float)controlY)};
    graphics.FillPolygon(g_HoverState == 3 ? &hoverBrush : &iconBrush, nextPts,
                         3);
    graphics.FillRectangle(g_HoverState == 3 ? &hoverBrush : &iconBrush, nX,
                           (float)controlY - (iconH / 2), 2.0f * (float)scale,
                           iconH);

    // 3. Text or Lyrics
    textX = (int)(nX + (20 * scale));
  }
  int textMaxW = width - textX - 10;

  // --- Draw Floating Lock Button ---
  if (g_Settings.displayMode == 1 && !g_Settings.isLocked) {
    float lockX = width - (16.0f * (float)scale);
    float lockY = height / 2.0f;

    SolidBrush lockBodyBrush(
        g_HoverState == 5
            ? Color(255, mainColor.GetRed(), mainColor.GetGreen(),
                    mainColor.GetBlue())
            : Color(160, mainColor.GetRed(), mainColor.GetGreen(),
                    mainColor.GetBlue()));
    Pen lockShacklePen(&lockBodyBrush, 2.0f * (float)scale);

    // Draw padlock body
    float bodyW = 10.0f * (float)scale;
    float bodyH = 8.0f * (float)scale;
    graphics.FillRectangle(&lockBodyBrush, lockX - (bodyW / 2), lockY - 1, bodyW,
                           bodyH);

    // Draw padlock shackle (arch)
    graphics.DrawArc(&lockShacklePen, lockX - (bodyW / 2) + 1,
                     lockY - (5.0f * (float)scale) - 1, bodyW - 2,
                     8.0f * (float)scale, 180, 180);

    // Reduce textMaxW so the lyrics don't overlap the lock button
    textMaxW -= (int)(24.0f * scale);
  }

  FontFamily *pFontFamily = new FontFamily(FONT_NAME, nullptr);
  if (pFontFamily->GetLastStatus() != Ok) {
    delete pFontFamily;
    pFontFamily = new FontFamily(L"Segoe UI", nullptr);
  }
  Font font(pFontFamily, (REAL)g_Settings.fontSize, FontStyleBold, UnitPixel);
  Font smallFont(pFontFamily, (REAL)g_Settings.fontSize - 2, FontStyleRegular,
                 UnitPixel);
  SolidBrush textBrush{mainColor};
  SolidBrush inactiveBrush{Color(128, mainColor.GetRed(), mainColor.GetGreen(),
                                 mainColor.GetBlue())};
  SolidBrush activeBrush{Color(255, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue())};

  ULONGLONG now = GetTickCount64();

  // --- MOVED 60FPS SYNC LOGIC HERE (Runs BEFORE visibility checks) ---
  long renderTimeMs = state.currentTimeMs;
  if (state.isPlaying) {
    renderTimeMs += (long)(now - state.lastUpdateTick);
  }

  if (state.hasMedia && !state.lyrics.empty()) {
      int bestIdx = -1;
      int nextIdx = -1;
      for (int i = 0; i < (int)state.lyrics.size(); i++) {
        if (renderTimeMs >= state.lyrics[i].startTimeMs) {
          bestIdx = i;
        } else {
          nextIdx = i;
          break;
        }
      }

      // --- Dynamic Adaptive Transition Algorithm ---
      if (bestIdx >= 0 && nextIdx >= 0) {
        long lineEnd = state.lyrics[bestIdx].startTimeMs + state.lyrics[bestIdx].durationMs;
        long nextStart = state.lyrics[nextIdx].startTimeMs;

        // 1. Ideal Transition: Start the crossfade exactly 300ms before the next line begins 
        // (Because the crossfade animation itself takes exactly 300ms to finish)
        long idealTransition = nextStart - 300;

        // 2. Fast-Song Constraint: Wait at least 50ms after the current line finishes 
        // so we don't accidentally cut off the last word while it's still being sung!
        long earliestTransition = lineEnd + 50;

        // 3. Adaptive Decision: Pick the safest time to transition
        long triggerTime = (idealTransition > earliestTransition) ? idealTransition : earliestTransition;

        if (renderTimeMs >= triggerTime) {
          bestIdx = nextIdx;
        }
      }

      // Apply the new index
      if (bestIdx != state.currentLineIndex && bestIdx >= 0) {
        lock_guard<mutex> guard(g_MediaState.lock);
        if (state.currentLineIndex >= 0 && state.currentLineIndex < (int)g_MediaState.lyrics.size()) {
          g_MediaState.prevLine = g_MediaState.lyrics[state.currentLineIndex];
          g_PrevLyricScrollOffset = g_LyricScrollOffset;
          g_MediaState.transitionStartTime = now;
        }
        g_MediaState.currentLineIndex = bestIdx;

        // Update local state so it draws instantly this frame
        state.currentLineIndex = bestIdx;
        state.prevLine = g_MediaState.prevLine;
        state.transitionStartTime = g_MediaState.transitionStartTime;
      }
  }
  // ----------------------------------

  bool isHoveredLine = (g_HoverState == 4);
  bool isPaused = !state.isPlaying;

  // Now it actually knows the correct index instead of being stuck at -1!
  bool hasLyrics = state.hasMedia && !state.lyrics.empty() && state.currentLineIndex != -1;

  // Logic: Force InfoMode if paused, hovered, or just changed songs
  // FIX: Skip the 1-second delay if the lyrics engine has already synced a valid line!
  bool forceInfo = isPaused || isHoveredLine ||
                   (now - g_MediaState.lastHoverOutTime < 1000) ||
                   ((now - g_MediaState.lastSongChangeTime < 1000) && state.currentLineIndex == -1);

  bool showLyrics = !forceInfo && hasLyrics;

  if (showLyrics) {
    // --- LyricMode ---
    StringFormat format(StringFormat::GenericTypographic());
    format.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces | format.GetFormatFlags());

    auto DrawSingleLine = [&](float startX, float alpha, LyricLine &lineData,
                              int scrollOffset) {
      if (lineData.text.empty())
        return;

      // Cap max opacity to 80% (204) if locked floating, else 100% (255)
      int maxAlpha = isLockedFloating ? 204 : 255;
      int aVal = (int)((float)maxAlpha * alpha);
      if (aVal <= 0)
        return;

      // --- FIX: HOLD THE ANIMATION ---
      // renderTimeMs includes the massive 750ms offset that makes the text appear early.
      // We strip that offset back out here so the fill and scrolling wait for the true audio!
      long syncTimeMs = renderTimeMs;
      if (lineData.words.empty()) {
          syncTimeMs -= (g_Settings.lineOffset - g_Settings.syllableOffset);
      }
      // -------------------------------

      SolidBrush aBrush(Color(aVal, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue())); // Active (Themed)
      SolidBrush iBrush(
          Color(128 * aVal / 255, mainColor.GetRed(), mainColor.GetGreen(),
                mainColor.GetBlue())); // Inactive (Gray)
      SolidBrush shadowBrush(Color((int)(aVal * 0.8f), 0, 0, 0)); // Black Shadow

      RectF mainBound, romBound, transBound;
      graphics.MeasureString(lineData.text.c_str(), -1, &font, PointF(0, 0),
                             &format, &mainBound);

      float totalHeight = mainBound.Height;
      bool hasRom =
          g_Settings.enableRomanization && !lineData.romanization.empty();
      bool hasTrans = g_Settings.enableTranslation &&
                      !g_Settings.translationLang.empty() &&
                      !lineData.translation.empty();

      if (hasRom) {
        graphics.MeasureString(lineData.romanization.c_str(), -1, &smallFont,
                               PointF(0, 0), &format, &romBound);
        totalHeight += romBound.Height;
      }
      if (hasTrans) {
        graphics.MeasureString(lineData.translation.c_str(), -1, &smallFont,
                               PointF(0, 0), &format, &transBound);
        totalHeight += transBound.Height;
      }

      float currentY = (g_Settings.height - totalHeight) / 2.0f;
      float dX = startX;

      bool shouldScroll = (mainBound.Width > textMaxW);
      if (shouldScroll) {
        float maxScroll = mainBound.Width - textMaxW + 15.0f; // 15px padding

        // Calculate exact progress percentage based on the audio timestamp
        float scrollProgress = 0.0f;
        if (syncTimeMs > lineData.startTimeMs && lineData.durationMs > 0) {
            scrollProgress = (float)(syncTimeMs - lineData.startTimeMs) / (float)lineData.durationMs;
        }

        // Clamp between 0% and 100%
        if (scrollProgress < 0.0f) scrollProgress = 0.0f;
        if (scrollProgress > 1.0f) scrollProgress = 1.0f;

        // Apply dynamic scroll position
        float sPos = maxScroll * scrollProgress;
        dX -= sPos;
      }

      if (!lineData.words.empty()) {
        // --- SYLLABLE SYNC (BetterLyrics) ---
        float curX = dX;
        for (auto &w : lineData.words) {
          RectF wRect;
          graphics.MeasureString(w.text.c_str(), -1, &font, PointF(0, 0),
                                 &format, &wRect);
          float wWidth = wRect.Width > 0.1f ? wRect.Width : 1.0f;

          float progress = 0.0f;
          if (syncTimeMs > w.startTimeMs && w.durationMs > 0) {
            progress = (float)(syncTimeMs - w.startTimeMs) / w.durationMs;
          }

          // Draw Drop Shadow Behind Syllables
          if (isLockedFloating) {
              graphics.DrawString(w.text.c_str(), -1, &font, PointF(curX + 2, currentY + 2), &format, &shadowBrush);
          }

          if (progress > 0.0f && progress < 1.0f) {
            Color c1(aVal, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue());
            Color c2(128 * aVal / 255, mainColor.GetRed(), mainColor.GetGreen(),
                    mainColor.GetBlue());
            LinearGradientBrush grad(PointF(curX, 0), PointF(curX + wWidth, 0),
                                     c1, c2);

            // Fixed Blend Math: White -> Gray
            float f[] = {0.0f, 0.0f, 1.0f, 1.0f};
            float pPos = progress + 0.001f;
            if (pPos > 1.0f)
              pPos = 1.0f;
            float p[] = {0.0f, progress, pPos, 1.0f};
            grad.SetBlend(f, p, 4);

            graphics.DrawString(w.text.c_str(), -1, &font,
                                PointF(curX, currentY), &format, &grad);
          } else if (progress >= 1.0f) {
            graphics.DrawString(w.text.c_str(), -1, &font,
                                PointF(curX, currentY), &format, &aBrush);
          } else {
            graphics.DrawString(w.text.c_str(), -1, &font,
                                PointF(curX, currentY), &format, &iBrush);
          }
          curX += wWidth;
        }
      } else {
        // --- LINE SYNC FALLBACK (LRCLib) ---
        float lineProgress = 0.0f;
        if (syncTimeMs > lineData.startTimeMs && lineData.durationMs > 0) {
          lineProgress =
              (float)(syncTimeMs - lineData.startTimeMs) / lineData.durationMs;
        }

        // Draw Drop Shadow
        if (isLockedFloating) {
            graphics.DrawString(lineData.text.c_str(), -1, &font, PointF(dX + 2, currentY + 2), &format, &shadowBrush);
        }

        if (lineProgress > 0.0f && lineProgress < 1.0f) {
          float bW = mainBound.Width > 0.1f ? mainBound.Width : 1.0f;
          Color c1(aVal, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue());
          Color c2(128 * aVal / 255, mainColor.GetRed(), mainColor.GetGreen(),
                  mainColor.GetBlue());
          LinearGradientBrush grad(PointF(dX, 0), PointF(dX + bW, 0), c1, c2);

          float f[] = {0.0f, 0.0f, 1.0f, 1.0f};
          float pPos = lineProgress + 0.001f;
          if (pPos > 1.0f)
            pPos = 1.0f;
          float p[] = {0.0f, lineProgress, pPos, 1.0f};
          grad.SetBlend(f, p, 4);

          graphics.DrawString(lineData.text.c_str(), -1, &font,
                              PointF(dX, currentY), &format, &grad);
        } else if (lineProgress >= 1.0f) {
          graphics.DrawString(lineData.text.c_str(), -1, &font,
                              PointF(dX, currentY), &format, &aBrush);
        } else {
          graphics.DrawString(lineData.text.c_str(), -1, &font,
                              PointF(dX, currentY), &format, &iBrush);
        }
      }

      // --- Draw Subtitles with Karaoke Animation ---
      currentY += mainBound.Height;

      // Calculate overall line progress for the subtitles
      float subProgress = 0.0f;
      if (syncTimeMs > lineData.startTimeMs && lineData.durationMs > 0) {
        subProgress = (float)(syncTimeMs - lineData.startTimeMs) / lineData.durationMs;
      }

      auto DrawAnimatedSubtitle = [&](const wstring& text, RectF& bound, Font& fontRef) {
          if (isLockedFloating) {
              graphics.DrawString(text.c_str(), -1, &fontRef, PointF(dX + 2, currentY + 2), &format, &shadowBrush);
          }

          if (subProgress > 0.0f && subProgress < 1.0f) {
              float bW = bound.Width > 0.1f ? bound.Width : 1.0f;
              Color c1(aVal, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue());
              Color c2(128 * aVal / 255, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue());
              LinearGradientBrush grad(PointF(dX, 0), PointF(dX + bW, 0), c1, c2);

              float f[] = {0.0f, 0.0f, 1.0f, 1.0f};
              float pPos = subProgress + 0.001f;
              if (pPos > 1.0f) pPos = 1.0f;
              float p[] = {0.0f, subProgress, pPos, 1.0f};
              grad.SetBlend(f, p, 4);

              graphics.DrawString(text.c_str(), -1, &fontRef, PointF(dX, currentY), &format, &grad);
          } else if (subProgress >= 1.0f) {
              graphics.DrawString(text.c_str(), -1, &fontRef, PointF(dX, currentY), &format, &aBrush);
          } else {
              graphics.DrawString(text.c_str(), -1, &fontRef, PointF(dX, currentY), &format, &iBrush);
          }
          currentY += bound.Height;
      };

      if (hasRom) {
          DrawAnimatedSubtitle(lineData.romanization, romBound, smallFont);
      }
      if (hasTrans) {
          DrawAnimatedSubtitle(lineData.translation, transBound, smallFont);
      }
    };

    Region lyricClip(Rect(textX, 0, textMaxW, height));
    graphics.SetClip(&lyricClip);

    const int TRANSITION_DURATION = 300;
    float transitionPos =
        (float)(now - state.transitionStartTime) / (float)TRANSITION_DURATION;
    if (transitionPos < 1.0f && !state.prevLine.text.empty()) {
      // Draw Previous line fading out
      DrawSingleLine((float)textX, 1.0f - transitionPos, state.prevLine,
                     g_PrevLyricScrollOffset);
      // Draw Current line fading in
      DrawSingleLine((float)textX, transitionPos,
                     state.lyrics[state.currentLineIndex], g_LyricScrollOffset);
    } else {
      DrawSingleLine((float)textX, 1.0f, state.lyrics[state.currentLineIndex],
                     g_LyricScrollOffset);
    }

    graphics.ResetClip();
  } else {
    // --- InfoMode ---
    wstring fullText = state.title;
    if (!state.artist.empty())
      fullText += L" • " + state.artist;

    RectF titleRect;
    graphics.MeasureString(fullText.c_str(), -1, &font, RectF(0, 0, 2000, 100),
                           &titleRect);

    float textY = (height - titleRect.Height) / 2.0f;
    Region textClip(Rect(textX, 0, textMaxW, height));
    graphics.SetClip(&textClip);

    // Apply Opacity limits and Setup shadow brushes
    int titleAlpha = isLockedFloating ? 204 : 255;
    SolidBrush textBrush{Color(titleAlpha, mainColor.GetRed(), mainColor.GetGreen(), mainColor.GetBlue())};
    SolidBrush shadowBrush{Color((int)(titleAlpha * 0.8f), 0, 0, 0)};

    if (titleRect.Width > textMaxW) {
      g_IsScrolling = true;
      g_TextWidth = (int)titleRect.Width;
      float drawX =
          (float)(textX - (g_ScrollOffset % (int)(titleRect.Width + 40)));

      // Shadow Pass
      if (isLockedFloating) {
          graphics.DrawString(fullText.c_str(), -1, &font, PointF(drawX + 2, textY + 2), &shadowBrush);
          if (drawX + titleRect.Width < textX + textMaxW)
              graphics.DrawString(fullText.c_str(), -1, &font, PointF(drawX + titleRect.Width + 40 + 2, textY + 2), &shadowBrush);
      }

      // Main Pass
      graphics.DrawString(fullText.c_str(), -1, &font, PointF(drawX, textY),
                          &textBrush);
      if (drawX + titleRect.Width < textX + textMaxW)
        graphics.DrawString(fullText.c_str(), -1, &font,
                            PointF(drawX + titleRect.Width + 40, textY),
                            &textBrush);
    } else {
      g_IsScrolling = false;
      g_ScrollOffset = 0;

      if (isLockedFloating) {
          graphics.DrawString(fullText.c_str(), -1, &font, PointF((float)textX + 2, textY + 2), &shadowBrush);
      }
      graphics.DrawString(fullText.c_str(), -1, &font,
                          PointF((float)textX, textY), &textBrush);
    }
    graphics.ResetClip();
  }
  delete pFontFamily;
}

// --- Event Hook ---
bool IsTaskbarWindow(HWND hwnd) {
  WCHAR cls[64];
  if (!hwnd)
    return false;
  GetClassNameW(hwnd, cls, ARRAYSIZE(cls));
  return wcscmp(cls, L"Shell_TrayWnd") == 0;
}

void CALLBACK TaskbarEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG,
                               LONG, DWORD, DWORD) {
  if (!IsTaskbarWindow(hwnd) || !g_hMediaWindow)
    return;
  PostMessage(g_hMediaWindow, WM_APP + 10, 0, 0);
}

// Register Event Hook scoped to Taskbar Thread
void RegisterTaskbarHook(HWND hwnd) {
  HWND hTaskbar = FindWindow(L"Shell_TrayWnd", nullptr);
  if (hTaskbar) {
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hTaskbar, &pid);
    if (tid != 0) {
      g_TaskbarHook = SetWinEventHook(
          EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr,
          TaskbarEventProc, pid, tid,
          WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    }
  }
  PostMessage(hwnd, WM_APP + 10, 0, 0);
}

// --- Window Procedure ---
#define IDT_POLL_MEDIA 1001
#define IDT_ANIMATION 1002
#define IDT_UI_UPDATE 1003
#define APP_WM_CLOSE WM_APP

// --- Robust Fullscreen Detection ---
bool IsForegroundFullscreen() {
  // 1. Fast check for standard D3D Games
  QUERY_USER_NOTIFICATION_STATE state;
  if (SUCCEEDED(SHQueryUserNotificationState(&state))) {
    if (state == QUNS_RUNNING_D3D_FULL_SCREEN ||
        state == QUNS_PRESENTATION_MODE) {
      return true;
    }
  }

  // 2. Physical geometry check for Borderless Windowed / Browser Fullscreen (F11)
  HWND fg = GetForegroundWindow();
  if (!fg || fg == GetDesktopWindow() || fg == GetShellWindow())
    return false;

  // Safely ignore the Desktop (Win+D) and Taskbar so lyrics stay visible
  WCHAR className[256];
  GetClassNameW(fg, className, 256);
  if (wcscmp(className, L"WorkerW") == 0 || wcscmp(className, L"Progman") == 0 ||
      wcscmp(className, L"Shell_TrayWnd") == 0) {
    return false;
  }

  // Measure the window against the monitor size
  RECT rcFg;
  if (GetWindowRect(fg, &rcFg)) {
    HMONITOR hMon = MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    if (GetMonitorInfoW(hMon, &mi)) {
      if (rcFg.left <= mi.rcMonitor.left && rcFg.top <= mi.rcMonitor.top &&
          rcFg.right >= mi.rcMonitor.right && rcFg.bottom >= mi.rcMonitor.bottom) {
        return true; // Window takes up the whole screen
      }
    }
  }
  return false;
}

// --- Window Procedure ---
LRESULT CALLBACK MediaWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                              LPARAM lParam) {
  switch (msg) {
  case WM_CREATE:
    UpdateAppearance(hwnd);
    SetTimer(hwnd, IDT_POLL_MEDIA, 1000, NULL);
    SetTimer(hwnd, IDT_UI_UPDATE, 16, NULL); // 60fps UI refresh
    RegisterTaskbarHook(hwnd);

    // Register hotkeys: D (Docked), F (Floating), S (Lock toggle)
    RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT | MOD_ALT, 'D');
    RegisterHotKey(hwnd, 2, MOD_CONTROL | MOD_SHIFT | MOD_ALT, 'F');
    RegisterHotKey(hwnd, 3, MOD_CONTROL | MOD_SHIFT | MOD_ALT, 'S');
    return 0;

  case WM_ERASEBKGND:
    return 1;

  case WM_CLOSE:
    return 0;

  case APP_WM_CLOSE:
    DestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    UnregisterHotKey(hwnd, 1);
    UnregisterHotKey(hwnd, 2);
    UnregisterHotKey(hwnd, 3);
    if (g_TaskbarHook) {
      UnhookWinEvent(g_TaskbarHook);
      g_TaskbarHook = nullptr;
    }
    g_SessionManager = nullptr;
    PostQuitMessage(0);
    return 0;

  case WM_SETTINGCHANGE:
    UpdateAppearance(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
    return 0;

  case WM_TIMER:
    if (wParam == IDT_POLL_MEDIA) {
      UpdateMediaInfo();

      bool shouldHide = false;

      // 1. Check Fullscreen with new robust physical check
      if (g_Settings.hideFullscreen) {
        if (IsForegroundFullscreen()) {
          shouldHide = true;
        }
      }

      // 2. Check Idle Timeout
      bool isPlaying = false;
      {
        lock_guard<mutex> guard(g_MediaState.lock);
        isPlaying = g_MediaState.isPlaying;
      }

      if (g_Settings.idleTimeout > 0) {
        if (isPlaying) {
          g_IdleSecondsCounter = 0;
          g_IsHiddenByIdle = false;
        } else {
          g_IdleSecondsCounter++;
          if (g_IdleSecondsCounter >= g_Settings.idleTimeout) {
            g_IsHiddenByIdle = true;
          }
        }
      } else {
        g_IsHiddenByIdle = false;
      }

      if (g_IsHiddenByIdle)
        shouldHide = true;

      // 3. Final Visibility Check (Using SetWindowPos for perfect DWM hiding)
      if (shouldHide && IsWindowVisible(hwnd)) {
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOACTIVATE);
      } else if (!shouldHide && !IsWindowVisible(hwnd)) {
        // FIX: Floating mode ignores taskbar visibility and forces itself topmost
        HWND hTaskbar = FindWindow(L"Shell_TrayWnd", nullptr);
        if (g_Settings.displayMode == 1 ||
            (hTaskbar && IsWindowVisible(hTaskbar))) {
          SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                       SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE |
                           SWP_NOACTIVATE);
        }
      }

      InvalidateRect(hwnd, NULL, FALSE);
    } else if (wParam == IDT_UI_UPDATE) {
      bool isPlaying = false;
      {
        lock_guard<mutex> guard(g_MediaState.lock);
        isPlaying = g_MediaState.isPlaying;
      }
      if (isPlaying) {
        InvalidateRect(hwnd, NULL, FALSE);
      }
    } else if (wParam == IDT_ANIMATION) {
      if (g_IsScrolling) {
        if (g_ScrollWait > 0) {
          g_ScrollWait--;
        } else {
          g_ScrollOffset++;
          if (g_ScrollOffset > g_TextWidth + 40) {
            g_ScrollOffset = 0;
            g_ScrollWait = 60;
          }
          InvalidateRect(hwnd, NULL, FALSE);
        }
      }

      // Handle Lyric Scroll Offset
      static int lastLineIdx = -1;
      int currentIdx = -1;
      {
        lock_guard<mutex> guard(g_MediaState.lock);
        currentIdx = g_MediaState.currentLineIndex;
      }
      if (currentIdx != lastLineIdx) {
        g_LyricScrollOffset = 0;
        lastLineIdx = currentIdx;
      }
      g_LyricScrollOffset++;
      InvalidateRect(hwnd, NULL, FALSE);
    }
    return 0;

  case WM_HOTKEY:
    if (wParam == 1) { // Docked Unlocked
      g_Settings.displayMode = 0;
      g_Settings.isLocked = false;
      UpdateAppearance(hwnd);
      PostMessage(hwnd, WM_APP + 10, 0, 0); // Force snap
      InvalidateRect(hwnd, NULL, TRUE);
    } else if (wParam == 2) { // Floating Unlocked
      g_Settings.displayMode = 1;
      g_Settings.isLocked = false;
      UpdateAppearance(hwnd);

      // FIX: Restore the last known floating position (if it exists)
      if (g_FloatX != -9999 && g_FloatY != -9999) {
        SetWindowPos(hwnd, HWND_TOPMOST, g_FloatX, g_FloatY, 0, 0,
                     SWP_NOSIZE | SWP_NOACTIVATE);
      }

      InvalidateRect(hwnd, NULL, TRUE);
    } else if (wParam == 3 && g_Settings.displayMode == 1) { // Lock toggle
      g_Settings.isLocked = !g_Settings.isLocked;
      UpdateAppearance(hwnd);
      InvalidateRect(hwnd, NULL, TRUE);
    }
    return 0;

  case WM_LBUTTONDOWN:
    // Allow dragging if Floating and Unlocked, BUT ONLY if we aren't clicking a
    // button
    if (g_Settings.displayMode == 1 && !g_Settings.isLocked) {
      if (g_HoverState == 0 || g_HoverState == 4) { // 0=BG, 4=Text
        ReleaseCapture();
        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
      }
    }
    return 0;

  case WM_EXITSIZEMOVE:
    // When the user finishes dragging, save the new X and Y coordinates
    if (g_Settings.displayMode == 1) {
      RECT rc;
      if (GetWindowRect(hwnd, &rc)) {
        g_FloatX = rc.left;
        g_FloatY = rc.top;
      }
    }
    return 0;

  case WM_APP + 10: {
    if (g_Settings.displayMode != 0)
      return 0; // Skip snapping if Floating

    HWND hTaskbar = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
    if (!hTaskbar)
      break;

    // Merged Logic: Check visibility first
    if (!IsWindowVisible(hTaskbar)) {
      if (IsWindowVisible(hwnd))
        ShowWindow(hwnd, SW_HIDE);
      return 0;
    }

    // Restore visibility if we aren't hidden by fullscreen or Idle modes
    if (!g_IsHiddenByIdle && !IsWindowVisible(hwnd)) {
      bool gameModeHide = false;
      if (g_Settings.hideFullscreen) {
        if (IsForegroundFullscreen())
          gameModeHide = true;
      }
      if (!gameModeHide) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      }
    }

    RECT rc;
    GetWindowRect(hTaskbar, &rc);

    int x = rc.left + g_Settings.offsetX;
    int taskbarHeight = rc.bottom - rc.top;
    int y = rc.top + (taskbarHeight / 2) - (g_Settings.height / 2) +
            g_Settings.offsetY;

    RECT myRc;
    GetWindowRect(hwnd, &myRc);
    if (myRc.left != x || myRc.top != y ||
        (myRc.right - myRc.left) != g_Settings.width ||
        (myRc.bottom - myRc.top) != g_Settings.height) {
      SetWindowPos(hwnd, HWND_TOPMOST, x, y, g_Settings.width,
                   g_Settings.height, SWP_NOACTIVATE);
    }
    return 0;
  }

  case WM_MOUSEMOVE: {
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    int artSize = g_Settings.height - 12;
    double scale = g_Settings.buttonScale;

    // Re-calculate hit targets based on scale
    int startControlX = 6 + artSize + (int)(12 * scale);
    float gap = 28.0f * (float)scale;
    float pX = (float)startControlX;
    float plX = pX + gap;
    float nX = plX + gap;
    float radius = 12.0f * (float)scale;

    int newState = 0;
    if (y > 6 && y < g_Settings.height - 6) {
      if (x >= pX - radius && x <= pX + radius)
        newState = 1;
      else if (x >= plX - radius && x <= plX + radius)
        newState = 2;
      else if (x >= nX - radius && x <= nX + radius)
        newState = 3;
      else
        newState = 4; // Text Area
    }

    // Lock Button Hitbox (Floating Mode Only, Far Right Edge)
    if (g_Settings.displayMode == 1 && !g_Settings.isLocked) {
      float lockX = g_Settings.width - (24.0f * (float)scale);
      if (x >= lockX && x <= g_Settings.width && y >= 0 &&
          y <= g_Settings.height) {
        newState = 5;
      }
    }

    if (newState != g_HoverState) {
      if (g_HoverState == 4 && newState != 4) {
        g_MediaState.lastHoverOutTime = GetTickCount64();
      }
      g_HoverState = newState;
      InvalidateRect(hwnd, NULL, FALSE);
    }

    TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0};
    TrackMouseEvent(&tme);
    return 0;
  }
  case WM_MOUSELEAVE:
    if (g_HoverState == 4) {
      g_MediaState.lastHoverOutTime = GetTickCount64();
    }
    g_HoverState = 0;
    InvalidateRect(hwnd, NULL, FALSE);
    break;
  case WM_LBUTTONUP:
    if (g_HoverState == 5) { // Clicked the Lock Button
      g_Settings.isLocked = true;
      UpdateAppearance(hwnd);
      InvalidateRect(hwnd, NULL, TRUE);
    } else if (g_HoverState > 0 && g_HoverState < 4) {
      SendMediaCommand(g_HoverState);
    }
    return 0;
  case WM_MOUSEWHEEL: {
    short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    keybd_event(zDelta > 0 ? VK_VOLUME_UP : VK_VOLUME_DOWN, 0, 0, 0);
    keybd_event(zDelta > 0 ? VK_VOLUME_UP : VK_VOLUME_DOWN, 0, KEYEVENTF_KEYUP,
                0);
    return 0;
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    RECT rc;
    GetClientRect(hwnd, &rc);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    DrawMediaPanel(memDC, rc.right, rc.bottom);

    if (g_IsScrolling)
      SetTimer(hwnd, IDT_ANIMATION, 16, NULL);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
    EndPaint(hwnd, &ps);
    return 0;
  }
  default:
    if (msg == g_TaskbarCreatedMsg) {
      if (g_TaskbarHook) {
        UnhookWinEvent(g_TaskbarHook);
        g_TaskbarHook = nullptr;
      }
      RegisterTaskbarHook(hwnd);
      return 0;
    }
    break;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

// --- Main Thread ---
void MediaThread() {
  winrt::init_apartment();

  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  WNDCLASS wc = {0};
  wc.lpfnWndProc = MediaWndProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = TEXT("WindhawkMusicLounge_GSMTC");
  wc.hCursor = LoadCursor(NULL, IDC_HAND);
  RegisterClass(&wc);

  // Try to use CreateWindowInBand
  HMODULE hUser32 = GetModuleHandle(L"user32.dll");
  pCreateWindowInBand CreateWindowInBand = nullptr;
  if (hUser32) {
    CreateWindowInBand =
        (pCreateWindowInBand)GetProcAddress(hUser32, "CreateWindowInBand");
  }

  if (CreateWindowInBand) {
    g_hMediaWindow = CreateWindowInBand(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        wc.lpszClassName, TEXT("MusicLounge"), WS_POPUP | WS_VISIBLE, 0, 0,
        g_Settings.width, g_Settings.height, NULL, NULL, wc.hInstance, NULL,
        ZBID_UIACCESS); // FIX: UI_ACCESS survives Win+D
    if (g_hMediaWindow) {
      Wh_Log(L"Created window in ZBID_UIACCESS band");
    }
  }

  if (!g_hMediaWindow) {
    Wh_Log(L"Falling back to CreateWindowEx");
    g_hMediaWindow = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        wc.lpszClassName, TEXT("MusicLounge"), WS_POPUP | WS_VISIBLE, 0, 0,
        g_Settings.width, g_Settings.height, NULL, NULL, wc.hInstance, NULL);
  }

  SetLayeredWindowAttributes(g_hMediaWindow, 0, 255, LWA_ALPHA);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnregisterClass(wc.lpszClassName, wc.hInstance);
  GdiplusShutdown(gdiplusToken);
  winrt::uninit_apartment();
}

std::thread *g_pMediaThread = nullptr;

// --- CALLBACKS ---
BOOL WhTool_ModInit() {
  SetCurrentProcessExplicitAppUserModelID(L"taskbar-music-lounge");
  LoadSettings();
  g_Running = true;
  g_pMediaThread = new std::thread(MediaThread);
  return TRUE;
}

void WhTool_ModUninit() {
  g_Running = false;
  if (g_hMediaWindow)
    SendMessage(g_hMediaWindow, APP_WM_CLOSE, 0, 0);
  if (g_pMediaThread) {
    if (g_pMediaThread->joinable())
      g_pMediaThread->join();
    delete g_pMediaThread;
    g_pMediaThread = nullptr;
  }
}

void WhTool_ModSettingsChanged() {
  LoadSettings();
  if (g_hMediaWindow) {
    SendMessage(g_hMediaWindow, WM_TIMER, IDT_POLL_MEDIA, 0);
    SendMessage(g_hMediaWindow, WM_SETTINGCHANGE, 0, 0);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Windhawk tool mod implementation for mods which don't need to inject to other
// processes or hook other functions. Context:
// https://github.com/ramensoftware/windhawk-mods/pull/1916
//
// The mod will load and run in a dedicated windhawk.exe process.
//
// Paste the code below as part of the mod code, and use these callbacks:
// * WhTool_ModInit
// * WhTool_ModSettingsChanged
// * WhTool_ModUninit
//
// Currently, other callbacks are not supported.

bool g_isToolModProcessLauncher;
HANDLE g_toolModProcessMutex;

void WINAPI EntryPoint_Hook() {
  Wh_Log(L">");
  ExitThread(0);
}

BOOL Wh_ModInit() {
  bool isService = false;
  bool isToolModProcess = false;
  bool isCurrentToolModProcess = false;
  int argc;
  LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);
  if (!argv) {
    Wh_Log(L"CommandLineToArgvW failed");
    return FALSE;
  }

  for (int i = 1; i < argc; i++) {
    if (wcscmp(argv[i], L"-service") == 0) {
      isService = true;
      break;
    }
  }

  for (int i = 1; i < argc - 1; i++) {
    if (wcscmp(argv[i], L"-tool-mod") == 0) {
      isToolModProcess = true;
      if (wcscmp(argv[i + 1], WH_MOD_ID) == 0) {
        isCurrentToolModProcess = true;
      }
      break;
    }
  }

  LocalFree(argv);

  if (isService) {
    return FALSE;
  }

  if (isCurrentToolModProcess) {
    g_toolModProcessMutex =
        CreateMutex(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
    if (!g_toolModProcessMutex) {
      Wh_Log(L"CreateMutex failed");
      ExitProcess(1);
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      Wh_Log(L"Tool mod already running (%s)", WH_MOD_ID);
      ExitProcess(1);
    }

    if (!WhTool_ModInit()) {
      ExitProcess(1);
    }

    IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *)GetModuleHandle(nullptr);
    IMAGE_NT_HEADERS *ntHeaders =
        (IMAGE_NT_HEADERS *)((BYTE *)dosHeader + dosHeader->e_lfanew);

    DWORD entryPointRVA = ntHeaders->OptionalHeader.AddressOfEntryPoint;
    void *entryPoint = (BYTE *)dosHeader + entryPointRVA;

    Wh_SetFunctionHook(entryPoint, (void *)EntryPoint_Hook, nullptr);
    return TRUE;
  }

  if (isToolModProcess) {
    return FALSE;
  }

  g_isToolModProcessLauncher = true;
  return TRUE;
}

void Wh_ModAfterInit() {
  if (!g_isToolModProcessLauncher) {
    return;
  }

  WCHAR currentProcessPath[MAX_PATH];
  switch (GetModuleFileName(nullptr, currentProcessPath,
                            ARRAYSIZE(currentProcessPath))) {
  case 0:
  case ARRAYSIZE(currentProcessPath):
    Wh_Log(L"GetModuleFileName failed");
    return;
  }

  WCHAR
  commandLine[MAX_PATH + 2 +
              (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
  swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"", currentProcessPath,
             WH_MOD_ID);

  HMODULE kernelModule = GetModuleHandle(L"kernelbase.dll");
  if (!kernelModule) {
    kernelModule = GetModuleHandle(L"kernel32.dll");
    if (!kernelModule) {
      Wh_Log(L"No kernelbase.dll/kernel32.dll");
      return;
    }
  }

  using CreateProcessInternalW_t = BOOL(WINAPI *)(
      HANDLE hUserToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
      LPSECURITY_ATTRIBUTES lpProcessAttributes,
      LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles,
      DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
      LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation,
      PHANDLE hRestrictedUserToken);
  CreateProcessInternalW_t pCreateProcessInternalW =
      (CreateProcessInternalW_t)GetProcAddress(kernelModule,
                                               "CreateProcessInternalW");
  if (!pCreateProcessInternalW) {
    Wh_Log(L"No CreateProcessInternalW");
    return;
  }

  STARTUPINFO si{
      .cb = sizeof(STARTUPINFO),
      .dwFlags = STARTF_FORCEOFFFEEDBACK,
  };
  PROCESS_INFORMATION pi;
  if (!pCreateProcessInternalW(nullptr, currentProcessPath, commandLine,
                               nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS,
                               nullptr, nullptr, &si, &pi, nullptr)) {
    Wh_Log(L"CreateProcess failed");
    return;
  }

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
}

void Wh_ModSettingsChanged() {
  if (g_isToolModProcessLauncher) {
    return;
  }

  WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
  if (g_isToolModProcessLauncher) {
    return;
  }

  WhTool_ModUninit();
  ExitProcess(0);
}