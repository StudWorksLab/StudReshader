/**
   StudReshader
   Real-time Graphics Overlay for Roblox
   Copyright (c) 2026 StudWorks Lab

   This software is licensed under the MIT License.
   See the LICENSE file in the repository for full terms.

   This is a large open source project made by StudWorks Lab.
   For more information visit our Discord: https://discord.gg/Q7wmA4tMmJ

   Features:
   Ground Mirror Reflections with adjustable intensity and roughness
   Bloom and God Rays for atmospheric lighting
   Depth of Field with customizable focus and blur
   AI Upscaling using RealESRGAN and QuickSR models
   Player Detection Radar using YOLO object detection(W.I.P.)
   Style Transfer using Stable Diffusion models
   Complete color grading suite with multiple themes
   Performance optimized rendering pipeline
   Crosshair overlay with multiple styles
   Comprehensive UI with real-time controls

   Requirements:
   Windows 10 or 11 (64 bit)
   DirectX 11.1 compatible GPU
   Roblox Player or Roblox Studio
   4GB RAM recommended for AI features
   NVIDIA or AMD GPU with DirectML or CUDA for AI acceleration
*/
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#ifdef _MSC_VER
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#endif
#include <windowsx.h>
#include <shellapi.h>
#include <commdlg.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dcomp.h>
#include <winhttp.h>
#ifdef _MSC_VER
#include <winver.h>
#pragma comment(lib,"version.lib")
#endif
#include <wincodec.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <deque>
#include <array>
#include <map>
#include <random>
#include <numeric>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <cwchar>
#include <cctype>
#include <cwctype>
#include <fstream>
#include <sstream>
#include <functional>
#include <system_error>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
namespace sd15 {
	inline void appendUtf8(std::string& out, unsigned cp) {
		if (cp < 0x80)out.push_back((char)cp);
		else if (cp < 0x800) {
			out.push_back((char)(0xC0 | (cp >> 6)));
			out.push_back((char)(0x80 | (cp & 0x3F)));
		}
		else if (cp < 0x10000) {
			out.push_back((char)(0xE0 | (cp >> 12)));
			out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
			out.push_back((char)(0x80 | (cp & 0x3F)));
		}
		else {
			out.push_back((char)(0xF0 | (cp >> 18)));
			out.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
			out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
			out.push_back((char)(0x80 | (cp & 0x3F)));
		}
	}
	struct Json {
		enum class T { Null, Bool, Num, Str, Arr, Obj }t = T::Null;
		bool b = false;
		double n = 0.0;
		std::string s;
		std::vector<Json> arr;
		std::vector<std::pair<std::string, Json>> obj;
		const Json* find(const std::string& key)const {
			if (t != T::Obj)return nullptr;
			for (auto& kv : obj)if (kv.first == key)return &kv.second;
			return nullptr;
		}
	};
	class JsonParser {
	public:
		explicit JsonParser(const std::string& src) : m_s(src) {}
		Json parse() {
			skipWs();
			Json v = parseValue();
			skipWs();
			return v;
		}
	private:
		const std::string& m_s;
		size_t p = 0;
		void skipWs() { while (p < m_s.size() && (m_s[p] == ' ' || m_s[p] == '\t' || m_s[p] == '\r' || m_s[p] == '\n'))++p; }
		bool eof()const { return p >= m_s.size(); }
		char peek()const { return eof() ? '\0' : m_s[p]; }
		Json parseValue() {
			skipWs();
			if (eof())return {};
			char c = peek();
			if (c == '{')return parseObj();
			if (c == '[')return parseArr();
			if (c == '"') { Json v;v.t = Json::T::Str;v.s = parseString();return v; }
			if (c == 't' || c == 'f') { Json v;v.t = Json::T::Bool;v.b = parseBool();return v; }
			if (c == 'n') { parseNull();return {}; }
			return parseNum();
		}
		Json parseObj() {
			Json v;v.t = Json::T::Obj;++p;skipWs();
			if (peek() == '}') { ++p;return v; }
			for (;;) {
				skipWs();
				std::string key = parseString();
				skipWs();
				if (peek() == ':')++p;
				v.obj.emplace_back(std::move(key), parseValue());
				skipWs();
				char c = peek();
				if (c == ',') { ++p;continue; }
				if (c == '}') { ++p;break; }
				break;
			}
			return v;
		}
		Json parseArr() {
			Json v;v.t = Json::T::Arr;++p;skipWs();
			if (peek() == ']') { ++p;return v; }
			for (;;) {
				v.arr.push_back(parseValue());
				skipWs();
				char c = peek();
				if (c == ',') { ++p;continue; }
				if (c == ']') { ++p;break; }
				break;
			}
			return v;
		}
		std::string parseString() {
			if (peek() == '"')++p;
			std::string out;
			while (!eof()) {
				unsigned char c = (unsigned char)m_s[p++];
				if (c == '"')break;
				if (c == '\\') {
					if (eof())break;
					char e = m_s[p++];
					switch (e) {
					case '"': out.push_back('"');break;
					case '\\': out.push_back('\\');break;
					case '/': out.push_back('/');break;
					case 'b': out.push_back('\b');break;
					case 'f': out.push_back('\f');break;
					case 'n': out.push_back('\n');break;
					case 'r': out.push_back('\r');break;
					case 't': out.push_back('\t');break;
					case 'u': {
						if (p + 4 <= m_s.size()) {
							unsigned cp = 0;
							for (int i = 0;i < 4;++i) {
								char h = m_s[p + i];
								cp <<= 4;
								if (h >= '0' && h <= '9')cp |= (unsigned)(h - '0');
								else if (h >= 'a' && h <= 'f')cp |= (unsigned)(h - 'a' + 10);
								else if (h >= 'A' && h <= 'F')cp |= (unsigned)(h - 'A' + 10);
							}
							p += 4;
							if (cp >= 0xD800 && cp <= 0xDBFF && p + 6 <= m_s.size() && m_s[p] == '\\' && m_s[p + 1] == 'u') {
								unsigned lo = 0;
								for (int i = 0;i < 4;++i) {
									char h = m_s[p + 2 + i];
									lo <<= 4;
									if (h >= '0' && h <= '9')lo |= (unsigned)(h - '0');
									else if (h >= 'a' && h <= 'f')lo |= (unsigned)(h - 'a' + 10);
									else if (h >= 'A' && h <= 'F')lo |= (unsigned)(h - 'A' + 10);
								}
								if (lo >= 0xDC00 && lo <= 0xDFFF) {
									cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
									p += 6;
								}
							}
							appendUtf8(out, cp);
						}
						break;
					}
					default: out.push_back(e);break;
					}
				}
				else {
					out.push_back((char)c);
				}
			}
			return out;
		}
		bool parseBool() {
			if (m_s.compare(p, 4, "true") == 0) { p += 4;return true; }
			if (m_s.compare(p, 5, "false") == 0) { p += 5;return false; }
			return false;
		}
		void parseNull() { if (m_s.compare(p, 4, "null") == 0)p += 4; }
		Json parseNum() {
			size_t start = p;
			if (peek() == '-')++p;
			while (!eof() && ((m_s[p] >= '0' && m_s[p] <= '9') || m_s[p] == '.' || m_s[p] == 'e' || m_s[p] == 'E' || m_s[p] == '+' || m_s[p] == '-'))++p;
			Json v;v.t = Json::T::Num;v.n = std::atof(m_s.substr(start, p - start).c_str());
			return v;
		}
	};
	inline unsigned decodeUtf8(const std::string& s, size_t& i) {
		unsigned char c0 = (unsigned char)s[i];
		if (c0 < 0x80) { ++i;return c0; }
		if ((c0 & 0xE0) == 0xC0 && i + 1 < s.size()) {
			unsigned cp = ((unsigned)(c0 & 0x1F) << 6) | ((unsigned char)s[i + 1] & 0x3F);
			i += 2;return cp;
		}
		if ((c0 & 0xF0) == 0xE0 && i + 2 < s.size()) {
			unsigned cp = ((unsigned)(c0 & 0x0F) << 12) | (((unsigned char)s[i + 1] & 0x3F) << 6) | ((unsigned char)s[i + 2] & 0x3F);
			i += 3;return cp;
		}
		if ((c0 & 0xF8) == 0xF0 && i + 3 < s.size()) {
			unsigned cp = ((unsigned)(c0 & 0x07) << 18) | (((unsigned char)s[i + 1] & 0x3F) << 12) | (((unsigned char)s[i + 2] & 0x3F) << 6) | ((unsigned char)s[i + 3] & 0x3F);
			i += 4;return cp;
		}
		++i;return c0;
	}
	inline bool isUnicodeLetter(unsigned cp) {
		if ((cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z'))return true;
		if ((cp >= 0x00C0 && cp <= 0x02FF))return true;
		if ((cp >= 0x0370 && cp <= 0x0482) || (cp >= 0x048A && cp <= 0x052F))return true;
		if ((cp >= 0x0531 && cp <= 0x058F))return true;
		if ((cp >= 0x05D0 && cp <= 0x05EA) || (cp >= 0x05EF && cp <= 0x05F2))return true;
		if ((cp >= 0x0620 && cp <= 0x064A) || (cp >= 0x066E && cp <= 0x06D3) || cp == 0x06D5 ||
			(cp >= 0x06FA && cp <= 0x06FC) || (cp >= 0x0710 && cp <= 0x072F) || (cp >= 0x074D && cp <= 0x07A5) || cp == 0x07B1)return true;
		if ((cp >= 0x0904 && cp <= 0x0939) || cp == 0x093D || cp == 0x0950 || (cp >= 0x0958 && cp <= 0x0961) ||
			(cp >= 0x0972 && cp <= 0x0980))return true;
		if ((cp >= 0x0985 && cp <= 0x09B9) || (cp >= 0x0A05 && cp <= 0x0A39) || (cp >= 0x0A85 && cp <= 0x0AB9) ||
			(cp >= 0x0B05 && cp <= 0x0B39) || (cp >= 0x0B85 && cp <= 0x0BB9) || (cp >= 0x0C05 && cp <= 0x0C39) ||
			(cp >= 0x0C85 && cp <= 0x0CB9) || (cp >= 0x0D05 && cp <= 0x0D39) || (cp >= 0x0D80 && cp <= 0x0DCF))return true;
		if ((cp >= 0x0E01 && cp <= 0x0E30) || (cp >= 0x0E32 && cp <= 0x0E33) || (cp >= 0x0E40 && cp <= 0x0E44))return true;
		if ((cp >= 0x0E81 && cp <= 0x0EB0) || (cp >= 0x0EB2 && cp <= 0x0EB3) || (cp >= 0x0EC0 && cp <= 0x0EC4))return true;
		if ((cp >= 0x0F00 && cp <= 0x0F47) || (cp >= 0x0F49 && cp <= 0x0F69))return true;
		if ((cp >= 0x1000 && cp <= 0x109F))return true;
		if ((cp >= 0x1780 && cp <= 0x17D3) || (cp >= 0x17D7 && cp <= 0x17DC))return true;
		if ((cp >= 0x1200 && cp <= 0x137F))return true;
		if ((cp >= 0x13A0 && cp <= 0x13F5))return true;
		if ((cp >= 0x1400 && cp <= 0x167F))return true;
		if ((cp >= 0x1100 && cp <= 0x11FF) || (cp >= 0x3130 && cp <= 0x318F))return true;
		if ((cp >= 0x3040 && cp <= 0x30FF) || (cp >= 0x31F0 && cp <= 0x31FF))return true;
		if ((cp >= 0x3400 && cp <= 0x4DBF) || (cp >= 0x4E00 && cp <= 0x9FFF) || (cp >= 0xAC00 && cp <= 0xD7AF))return true;
		if ((cp >= 0x2E80 && cp <= 0x2FDF))return true;
		if ((cp >= 0xA000 && cp <= 0xA48F))return true;
		if ((cp >= 0xF900 && cp <= 0xFAFF))return true;
		if ((cp >= 0xFF21 && cp <= 0xFF3A) || (cp >= 0xFF41 && cp <= 0xFF5A))return true;
		if ((cp >= 0x1D400 && cp <= 0x1D7FF))return true;
		if ((cp >= 0x10A0 && cp <= 0x10C5))return true;
		return false;
	}
	inline bool isUnicodeDigit(unsigned cp) {
		if (cp >= '0' && cp <= '9')return true;
		if ((cp >= 0x0660 && cp <= 0x0669) || (cp >= 0x06F0 && cp <= 0x06F9))return true;
		if ((cp >= 0x0966 && cp <= 0x096F) || (cp >= 0x0E50 && cp <= 0x0E59) || (cp >= 0x0ED0 && cp <= 0x0ED9))return true;
		if ((cp >= 0xFF10 && cp <= 0xFF19))return true;
		return false;
	}
	inline bool isWhitespaceCp(unsigned cp) {
		return cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r' || cp == '\f' || cp == '\v' || cp == 0x00A0 || cp == 0x3000;
	}
	inline std::string toLowerUtf8(const std::string& in) {
		std::string out;
		out.reserve(in.size());
		size_t i = 0;
		while (i < in.size()) {
			unsigned char c = (unsigned char)in[i];
			if (c < 0x80) {
				out.push_back((char)std::tolower(c));
				++i;
			}
			else {
				size_t j = i;
				unsigned cp = decodeUtf8(in, j);
				unsigned lc = cp;
				if (cp >= 'A' && cp <= 'Z')lc = cp + 0x20;
				else if ((cp >= 0x00C0 && cp <= 0x00DE) && cp != 0x00D7)lc = cp + 0x20;
				else if (cp >= 0x0391 && cp <= 0x03A9 && cp != 0x03A2)lc = cp + 0x20;
				else if (cp >= 0x0410 && cp <= 0x042F)lc = cp + 0x20;
				else if (cp >= 0xFF21 && cp <= 0xFF3A)lc = cp + 0x20;
				if (lc != cp)appendUtf8(out, lc);else out.append(in, i, j - i);
				i = j;
			}
		}
		return out;
	}
	inline std::string htmlUnescape(const std::string& in) {
		std::string out;
		out.reserve(in.size());
		size_t i = 0;
		while (i < in.size()) {
			if (in[i] == '&') {
				size_t semi = in.find(';', i);
				if (semi != std::string::npos && semi - i <= 12) {
					std::string ent = in.substr(i + 1, semi - i - 1);
					std::string rep;
					if (ent == "amp")rep = "&";
					else if (ent == "lt")rep = "<";
					else if (ent == "gt")rep = ">";
					else if (ent == "quot")rep = "\"";
					else if (ent == "apos")rep = "'";
					else if (ent == "nbsp")rep = " ";
					else if (!ent.empty() && ent[0] == '#') {
						unsigned cp = 0;
						bool hex = ent.size() > 1 && (ent[1] == 'x' || ent[1] == 'X');
						size_t k = hex ? 2 : 1;
						bool ok = !hex || ent.size() > 2;
						for (;ok && k < ent.size();++k) {
							char h = ent[k];
							unsigned d;
							if (h >= '0' && h <= '9')d = (unsigned)(h - '0');
							else if (hex && h >= 'a' && h <= 'f')d = (unsigned)(h - 'a' + 10);
							else if (hex && h >= 'A' && h <= 'F')d = (unsigned)(h - 'A' + 10);
							else { ok = false;break; }
							cp = cp * (hex ? 16u : 10u) + d;
						}
						if (ok)appendUtf8(rep, cp);
					}
					if (!rep.empty()) { out += rep;i = semi + 1;continue; }
				}
			}
			out.push_back(in[i]);
			++i;
		}
		return out;
	}
	inline std::string whitespaceClean(const std::string& in) {
		std::string out;
		out.reserve(in.size());
		bool pending = false;
		for (size_t i = 0;i < in.size();) {
			size_t j = i;
			unsigned cp = decodeUtf8(in, j);
			if (isWhitespaceCp(cp)) { pending = !out.empty();i = j;continue; }
			if (pending) { out.push_back(' ');pending = false; }
			out.append(in, i, j - i);
			i = j;
		}
		return out;
	}
	inline std::vector<std::string> buildBytesToUnicode() {
		std::vector<int> bs;
		for (int b = '!';b <= '~';++b)bs.push_back(b);
		for (int b = 0xA1;b <= 0xAC;++b)bs.push_back(b);
		for (int b = 0xAE;b <= 0xFF;++b)bs.push_back(b);
		std::vector<int> cs = bs;
		int n = 0;
		for (int b = 0;b < 256;++b) {
			if (std::find(bs.begin(), bs.end(), b) == bs.end()) {
				bs.push_back(b);
				cs.push_back(256 + n);
				++n;
			}
		}
		std::vector<std::string> table(256);
		for (size_t k = 0;k < bs.size();++k) {
			std::string u;
			appendUtf8(u, (unsigned)cs[k]);
			table[(unsigned char)bs[k]] = u;
		}
		return table;
	}
	class ClipSimpleTokenizer {
	public:
		bool Load(const std::string& vocabPath, const std::string& mergesPath, std::string& err) {
			err.clear();
			std::string vsrc = ReadFile(vocabPath);
			if (vsrc.empty()) { err = "vocab.json missing or empty";return false; }
			Json doc = JsonParser(vsrc).parse();
			if (doc.t != Json::T::Obj) { err = "vocab.json is not a JSON object";return false; }
			m_encoder.reserve(doc.obj.size());
			for (auto& kv : doc.obj) {
				if (kv.second.t == Json::T::Num)
					m_encoder[kv.first] = (int32_t)kv.second.n;
			}
			if (m_encoder.size() < 49000) { err = "vocab.json looks truncated";return false; }
			std::string msrc = ReadFile(mergesPath);
			if (msrc.empty()) { err = "merges.txt missing or empty";return false; }
			m_bpeRanks.clear();
			size_t pos = 0;
			int lineNo = 0;
			while (pos < msrc.size()) {
				size_t nl = msrc.find('\n', pos);
				std::string line = msrc.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos);
				pos = (nl == std::string::npos) ? msrc.size() : nl + 1;
				++lineNo;
				if (lineNo == 1 && line.find("#version") != std::string::npos)continue;
				if (!line.empty() && line.back() == '\r')line.pop_back();
				if (line.empty())continue;
				size_t sp = line.find(' ');
				if (sp == std::string::npos || sp == 0 || sp + 1 >= line.size())continue;
				std::string a = line.substr(0, sp);
				std::string b = line.substr(sp + 1);
				m_bpeRanks[std::make_pair(std::move(a), std::move(b))] = (int)m_bpeRanks.size();
			}
			if (m_bpeRanks.size() < 48000) { err = "merges.txt looks truncated";return false; }
			m_byteEncoder = buildBytesToUnicode();
			m_cache.clear();
			m_cache["<|startoftext|>"] = "<|startoftext|>";
			m_cache["<|endoftext|>"] = "<|endoftext|>";
			return true;
		}
		std::vector<int32_t> EncodeRaw(const std::string& text)const {
			std::string t = toLowerUtf8(whitespaceClean(htmlUnescape(text)));
			std::vector<int32_t> ids;
			size_t i = 0;
			while (i < t.size()) {
				if (t.compare(i, 15, "<|startoftext|>") == 0) {
					auto it = m_encoder.find("<|startoftext|>");
					if (it != m_encoder.end())ids.push_back(it->second);
					i += 15;
					continue;
				}
				if (t.compare(i, 13, "<|endoftext|>") == 0) {
					auto it = m_encoder.find("<|endoftext|>");
					if (it != m_encoder.end())ids.push_back(it->second);
					i += 13;
					continue;
				}
				size_t j = i;
				unsigned cp = decodeUtf8(t, j);
				if (isWhitespaceCp(cp)) { i = j;continue; }
				if (cp == '\'') {
					static const char* kCont[] = { "'s","'t","'re","'ve","'m","'ll","'d" };
					bool matched = false;
					for (const char* c : kCont) {
						size_t cl = strlen(c);
						if (t.compare(i, cl, c) == 0) {
							AppendTokenIds(ids, t.substr(i, cl));
							i += cl;
							matched = true;
							break;
						}
					}
					if (matched)continue;
				}
				if (isUnicodeLetter(cp)) {
					size_t run = i;
					while (run < t.size()) {
						size_t k = run;
						unsigned c2 = decodeUtf8(t, k);
						if (!isUnicodeLetter(c2))break;
						run = k;
					}
					AppendTokenIds(ids, t.substr(i, run - i));
					i = run;
					continue;
				}
				if (isUnicodeDigit(cp)) {
					AppendTokenIds(ids, t.substr(i, j - i));
					i = j;
					continue;
				}
				{
					size_t run = i;
					while (run < t.size()) {
						size_t k = run;
						unsigned c2 = decodeUtf8(t, k);
						if (isWhitespaceCp(c2) || isUnicodeLetter(c2) || isUnicodeDigit(c2))break;
						if (c2 == '\'') {
							static const char* kCont2[] = { "'s","'t","'re","'ve","'m","'ll","'d" };
							bool isCont = false;
							for (const char* c : kCont2)if (t.compare(k, strlen(c), c) == 0) { isCont = true;break; }
							if (isCont)break;
						}
						run = k;
					}
					AppendTokenIds(ids, t.substr(i, run - i));
					i = run;
				}
			}
			return ids;
		}
		std::vector<int32_t> EncodeFull(const std::string& text, int maxLen = 77)const {
			std::vector<int32_t> ids = EncodeRaw(text);
			std::vector<int32_t> out;
			out.reserve(maxLen);
			out.push_back(BosId());
			int room = maxLen - 2;
			for (int k = 0;k < room && k < (int)ids.size();++k)out.push_back(ids[k]);
			out.push_back(EosId());
			while ((int)out.size() < maxLen)out.push_back(EosId());
			return out;
		}
		int32_t BosId()const { auto it = m_encoder.find("<|startoftext|>");return it == m_encoder.end() ? 49406 : it->second; }
		int32_t EosId()const { auto it = m_encoder.find("<|endoftext|>");return it == m_encoder.end() ? 49407 : it->second; }
	private:
		static std::string ReadFile(const std::string& path) {
			std::string out;
			FILE* f = nullptr;
			fopen_s_(&f, path.c_str(), "rb");
			if (!f)return out;
			std::vector<char> buf(65536);
			size_t n;
			while ((n = fread(buf.data(), 1, buf.size(), f)) > 0)out.append(buf.data(), n);
			fclose(f);
			return out;
		}
		static void fopen_s_(FILE** f, const char* path, const char* mode) {
#ifdef _MSC_VER
			fopen_s(f, path, mode);
#else
			* f = fopen(path, mode);
#endif
		}
		int32_t EncodeToken(const std::string& token)const {
			std::string enc;
			for (unsigned char c : token)enc += m_byteEncoder[c];
			std::string bpeOut = Bpe(enc);
			auto it = m_encoder.find(bpeOut);
			if (it != m_encoder.end())return it->second;
			size_t sp = 0;
			while (sp < bpeOut.size()) {
				size_t nxt = bpeOut.find(' ', sp);
				std::string piece = bpeOut.substr(sp, nxt == std::string::npos ? std::string::npos : nxt - sp);
				auto it2 = m_encoder.find(piece);
				if (it2 != m_encoder.end())return it2->second;
				sp = (nxt == std::string::npos) ? bpeOut.size() : nxt + 1;
			}
			return EosId();
		}
		void AppendTokenIds(std::vector<int32_t>& ids, const std::string& token)const {
			std::string enc;
			for (unsigned char c : token)enc += m_byteEncoder[c];
			std::string bpeOut = Bpe(enc);
			size_t sp = 0;
			while (sp < bpeOut.size()) {
				size_t nxt = bpeOut.find(' ', sp);
				std::string piece = bpeOut.substr(sp, nxt == std::string::npos ? std::string::npos : nxt - sp);
				if (!piece.empty()) {
					auto it = m_encoder.find(piece);
					if (it != m_encoder.end())ids.push_back(it->second);
				}
				sp = (nxt == std::string::npos) ? bpeOut.size() : nxt + 1;
			}
		}
		std::string Bpe(const std::string& token)const {
			auto cit = m_cache.find(token);
			if (cit != m_cache.end())return cit->second;
			std::vector<std::string> chars;
			{
				size_t k = 0;
				while (k < token.size()) {
					size_t nxt = k;
					decodeUtf8(token, nxt);
					chars.push_back(token.substr(k, nxt - k));
					k = nxt;
				}
			}
			if (chars.empty())return "";
			std::vector<std::string> word;
			for (size_t i = 0;i + 1 < chars.size();++i)word.push_back(chars[i]);
			word.push_back(chars.back() + "</w>");
			if (word.size() == 1) { m_cache[token] = token + "</w>";return token + "</w>"; }
			auto getPairs = [](const std::vector<std::string>& w) {
				std::set<std::pair<std::string, std::string>> pairs;
				for (size_t k = 0;k + 1 < w.size();++k)pairs.insert(std::make_pair(w[k], w[k + 1]));
				return pairs;
				};
			std::set<std::pair<std::string, std::string>> pairs = getPairs(word);
			while (true) {
				std::pair<std::string, std::string> best;
				int bestRank = INT32_MAX;
				bool found = false;
				for (auto& pr : pairs) {
					auto it = m_bpeRanks.find(pr);
					if (it != m_bpeRanks.end() && it->second < bestRank) { bestRank = it->second;best = pr;found = true; }
				}
				if (!found)break;
				std::vector<std::string> newWord;
				size_t i = 0;
				while (i < word.size()) {
					size_t j = i;
					while (j < word.size() && word[j] != best.first)++j;
					if (j == word.size()) {
						for (size_t k = i;k < word.size();++k)newWord.push_back(word[k]);
						break;
					}
					for (size_t k = i;k < j;++k)newWord.push_back(word[k]);
					i = j;
					if (i + 1 < word.size() && word[i] == best.first && word[i + 1] == best.second) {
						newWord.push_back(best.first + best.second);
						i += 2;
					}
					else {
						newWord.push_back(word[i]);
						i += 1;
					}
				}
				word = std::move(newWord);
				if (word.size() == 1)break;
				pairs = getPairs(word);
			}
			std::string joined;
			for (size_t k = 0;k < word.size();++k) {
				if (k)joined.push_back(' ');
				joined += word[k];
			}
			m_cache[token] = joined;
			return joined;
		}
		mutable std::unordered_map<std::string, int32_t> m_encoder;
		std::map<std::pair<std::string, std::string>, int> m_bpeRanks;
		std::vector<std::string> m_byteEncoder;
		mutable std::unordered_map<std::string, std::string> m_cache;
	};
	class DdimScheduler {
	public:
		static constexpr int kTrainSteps = 1000;
		static constexpr float kBetaStart = 0.00085f;
		static constexpr float kBetaEnd = 0.012f;
		DdimScheduler() {
			std::vector<float> betas(kTrainSteps);
			const float s0 = std::sqrt(kBetaStart);
			const float s1 = std::sqrt(kBetaEnd);
			for (int i = 0;i < kTrainSteps;++i) {
				float t = (float)i / (float)(kTrainSteps - 1);
				float s = s0 + (s1 - s0) * t;
				betas[i] = s * s;
			}
			m_alphasCumprod.resize(kTrainSteps);
			float prod = 1.0f;
			for (int i = 0;i < kTrainSteps;++i) {
				prod *= (1.0f - betas[i]);
				m_alphasCumprod[i] = prod;
			}
		}
		std::vector<int> Timesteps(int numSteps)const {
			std::vector<int> ts(numSteps);
			int step = kTrainSteps / numSteps;
			for (int i = 0;i < numSteps;++i)ts[i] = (int)std::lround((float)i * step);
			std::reverse(ts.begin(), ts.end());
			return ts;
		}
		void Step(const float* sample, const float* eps, float* out, int n, int t, int prevT)const {
			float at = m_alphasCumprod[t];
			float atPrev = (prevT >= 0) ? m_alphasCumprod[prevT] : 1.0f;
			float sqrtAt = std::sqrt(std::max(at, 1e-8f));
			float sqrtOneMinusAt = std::sqrt(std::max(1.0f - at, 0.0f));
			float sqrtAtPrev = std::sqrt(std::max(atPrev, 1e-8f));
			float sqrtOneMinusAtPrev = std::sqrt(std::max(1.0f - atPrev, 0.0f));
			for (int i = 0;i < n;++i) {
				float x0 = (sample[i] - sqrtOneMinusAt * eps[i]) / sqrtAt;
				out[i] = sqrtAtPrev * x0 + sqrtOneMinusAtPrev * eps[i];
			}
		}
		float AlphaCumprod(int t)const { return m_alphasCumprod[t]; }
	private:
		std::vector<float> m_alphasCumprod;
	};
	class LcmScheduler {
	public:
		static constexpr int kTrainSteps = 1000;
		static constexpr int kOriginalSteps = 50;
		static constexpr float kTimestepScaling = 10.0f;
		static constexpr float kSigmaData = 0.5f;
		LcmScheduler() {
			const float s0 = std::sqrt(0.00085f);
			const float s1 = std::sqrt(0.012f);
			std::vector<float> betas(kTrainSteps);
			for (int i = 0;i < kTrainSteps;++i) {
				float t = (float)i / (float)(kTrainSteps - 1);
				float s = s0 + (s1 - s0) * t;
				betas[i] = s * s;
			}
			m_acp.resize(kTrainSteps);
			float prod = 1.0f;
			for (int i = 0;i < kTrainSteps;++i) {
				prod *= (1.0f - betas[i]);
				m_acp[i] = prod;
			}
		}
		std::vector<int> Timesteps(int numInferenceSteps)const {
			std::vector<int> ts(numInferenceSteps);
			for (int i = 0;i < numInferenceSteps;++i) {
				int idx = (int)std::floor((double)i * (double)kOriginalSteps / (double)numInferenceSteps);
				ts[i] = 999 - idx * 20;
			}
			return ts;
		}
		static int StartIndex(int steps, float strength) {
			int init = std::min((int)std::lround((double)steps * (double)strength), steps);
			return std::max(steps - init, 0);
		}
		void Step(const float* sample, const float* eps, const float* noise, float* out,
			int n, int t, int prevT, bool lastStep)const {
			float at = m_acp[t];
			float atPrev = (prevT >= 0) ? m_acp[prevT] : m_acp[0];
			float bt = 1.0f - at;
			float scaled = (float)t * kTimestepScaling;
			float cSkip = (kSigmaData * kSigmaData) / (scaled * scaled + kSigmaData * kSigmaData);
			float cOut = scaled / std::sqrt(scaled * scaled + kSigmaData * kSigmaData);
			float sqAt = std::sqrt(std::max(at, 1e-8f));
			float sqBt = std::sqrt(std::max(bt, 0.0f));
			float sqAtP = std::sqrt(std::max(atPrev, 1e-8f));
			float sqBtP = std::sqrt(std::max(1.0f - atPrev, 0.0f));
			for (int i = 0;i < n;++i) {
				float x0 = (sample[i] - sqBt * eps[i]) / sqAt;
				float denoised = cOut * x0 + cSkip * sample[i];
				if (!lastStep)out[i] = sqAtP * denoised + sqBtP * noise[i];
				else out[i] = denoised;
			}
		}
		float AlphaCumprod(int t)const { return m_acp[t]; }
	private:
		std::vector<float> m_acp;
	};
}
#if __has_include(<onnxruntime_cxx_api.h>)&& __has_include(<dml_provider_factory.h>)
#include <onnxruntime_cxx_api.h>
#include <dml_provider_factory.h>
#define SR_HAS_ONNX 1
#else
#define SR_HAS_ONNX 0
#endif
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dcomp.lib")
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"comdlg32.lib")
#pragma comment(lib,"windowscodecs.lib")
#if SR_HAS_ONNX
#pragma comment(lib,"onnxruntime.lib")
#endif
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x11
#endif
using Microsoft::WRL::ComPtr;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static constexpr wchar_t kWindowClass[] = L"StudReshaderWindowClass";
static constexpr wchar_t kMutexName[] = L"Local\\StudReshader_SingleInstance";
static HWND g_overlayHwnd = nullptr;
static HWND g_robloxHwnd = nullptr;
static DWORD g_robloxPid = 0;
static HMONITOR g_captureMonitor = nullptr;
static HMONITOR g_targetMonitor = nullptr;
static int g_capRetryCounter = 0;
static int g_retargetCounter = 0;
static bool g_overlayVisible = false;
static HANDLE g_singleInstanceMutex = nullptr;
static ID3D11Device* g_dev = nullptr;
static std::string g_gpuName;
static bool g_nvidiaGpu = false;
static int g_nvMaj = 0, g_nvMin = 0;
static bool g_gpuDied = false;
static bool g_gpuInfoChecked = false;
static ID3D11DeviceContext* g_ctx = nullptr;
static IDXGISwapChain1* g_sc = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static IDCompositionDevice* g_dcd = nullptr;
static IDCompositionTarget* g_dct = nullptr;
static IDCompositionVisual* g_dcv = nullptr;
static ComPtr<IDXGIAdapter1> g_dxgiAdapter;
static UINT g_W = 1920, g_H = 1080;
static RECT g_panelRect = {};
static bool g_appRunning = true;
static bool g_showMenu = true;
static bool g_drawerOpen = true;
static bool g_compactMode = false;
static bool g_runtimeLossHandled = false;
static bool g_wantTextInput = false;
static bool g_mouseInPanel = false;
static int g_activeSec = 0;
static int g_prevSec = -1;
static float g_secAlpha = 1.0f;
static std::string g_aiStatus = "GPU Mode Ready";
static bool g_showSplash = true;
static float g_splashAlpha = 1.0f;
static float g_splashIntro = 0.0f;
static auto g_startTime = std::chrono::steady_clock::now();
static bool g_linkCopied = false;
static float g_linkCopiedTimer = 0.0f;
static bool g_showCloseToast = false;
static float g_closeToastTimer = 0.0f;
static float g_fpsHist[160] = {};
static int g_fpsPos = 0;
static char g_searchBuf[128] = "";
static ID3D11ShaderResourceView* g_logoSrv = nullptr;
static int g_logoW = 0, g_logoH = 0;
static ID3D11ShaderResourceView* g_discordSrv = nullptr;
static int g_discordW = 0, g_discordH = 0;
static std::mutex g_ctxMtx;
static int g_guiTheme = 3;
static ImU32 g_accentCol = IM_COL32(255, 140, 0, 255);
static ImU32 g_accentDimCol = IM_COL32(180, 70, 0, 255);
static ImVec2 g_lastExpandedMenuSize = ImVec2(980.0f, 700.0f);
#if SR_HAS_ONNX
static Ort::Env* g_ort_env = nullptr;
static Ort::Env& GetOrtEnv() {
	if (!g_ort_env)g_ort_env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "StudReshaderEnv");
	return *g_ort_env;
}
#endif
enum class GpuVendor { NVIDIA, AMD, INTEL, UNKNOWN };
struct GpuInfo {
	std::wstring name;
	GpuVendor vendor = GpuVendor::UNKNOWN;
	SIZE_T vramBytes = 0;
	UINT vendorId = 0;
	UINT deviceId = 0;
};
static GpuVendor VendorFromId(UINT id) {
	switch (id) {
	case 0x10DE: return GpuVendor::NVIDIA;
	case 0x1002: return GpuVendor::AMD;
	case 0x8086: return GpuVendor::INTEL;
	default: return GpuVendor::UNKNOWN;
	}
}
static std::vector<GpuInfo> EnumerateGpus() {
	std::vector<GpuInfo> gpus;
	ComPtr<IDXGIFactory6> fac;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&fac))))return gpus;
	ComPtr<IDXGIAdapter1> a;
	for (UINT i = 0;fac->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&a)) != DXGI_ERROR_NOT_FOUND;++i) {
		DXGI_ADAPTER_DESC1 d{};a->GetDesc1(&d);
		if (d.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { a.Reset();continue; }
		GpuInfo info;info.name = d.Description;info.vendorId = d.VendorId;info.deviceId = d.DeviceId;
		info.vendor = VendorFromId(d.VendorId);info.vramBytes = d.DedicatedVideoMemory;
		gpus.push_back(std::move(info));a.Reset();
	}
	return gpus;
}
static void ShowNativeError(const wchar_t* title, const wchar_t* text) {
	MessageBoxW(nullptr, text, title, MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
}
static void EnablePerMonitorDpiAwareness() {
	HMODULE h = LoadLibraryW(L"user32.dll");if (!h)return;
	typedef BOOL(WINAPI* SetDpiCtxFn)(HANDLE);SetDpiCtxFn setCtx = (SetDpiCtxFn)GetProcAddress(h, "SetProcessDpiAwarenessContext");
	if (setCtx) { if (setCtx((HANDLE)-4)) { FreeLibrary(h);return; } }
	typedef HRESULT(WINAPI* SetDpiAwareFn)(int);SetDpiAwareFn setAware = (SetDpiAwareFn)GetProcAddress(h, "SetProcessDpiAwareness");
	if (setAware)setAware(2);FreeLibrary(h);
}
struct RobloxWinCandidate { HWND hwnd = nullptr;LONG area = 0;bool isGameExe = false; };
static bool FileNameEqualsNoCase(const std::wstring& path, const wchar_t* target) {
	size_t slash = path.find_last_of(L"\\/");std::wstring file = (slash == std::wstring::npos) ? path : path.substr(slash + 1);
	size_t tl = wcslen(target);if (file.size() != tl)return false;
	for (size_t i = 0;i < tl;++i)if (towlower(file[i]) != towlower(target[i]))return false;
	return true;
}
static BOOL CALLBACK EnumRobloxProc(HWND hwnd, LPARAM lp) {
	if (!IsWindowVisible(hwnd))return TRUE;if (IsIconic(hwnd))return TRUE;
	wchar_t t[256] = {};if (GetWindowTextW(hwnd, t, 256) <= 0)return TRUE;
	std::wstring s = t;
	bool titleMatch = (s.find(L"Roblox") != std::wstring::npos);
	DWORD pid = 0;GetWindowThreadProcessId(hwnd, &pid);bool isGame = false;
	if (pid) {
		HANDLE hp = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);if (hp) {
			wchar_t img[1024] = {};DWORD isz = 1024;
			if (QueryFullProcessImageNameW(hp, 0, img, &isz))
				isGame = FileNameEqualsNoCase(img, L"RobloxPlayerBeta.exe") || FileNameEqualsNoCase(img, L"RobloxPlayer.exe") || FileNameEqualsNoCase(img, L"RobloxStudioBeta.exe");
			CloseHandle(hp);
		}
	}
	if (!isGame && !titleMatch)return TRUE;
	RECT r{};if (!GetWindowRect(hwnd, &r))return TRUE;LONG area = (r.right - r.left) * (r.bottom - r.top);
	auto* best = reinterpret_cast<RobloxWinCandidate*>(lp);
	if (best->isGameExe && !isGame)return TRUE;
	if (!best->isGameExe && isGame) { best->hwnd = hwnd;best->area = area;best->isGameExe = true;return TRUE; }
	if (area > best->area) { best->hwnd = hwnd;best->area = area; }return TRUE;
}
static HWND FindRobloxWindow() { RobloxWinCandidate best;EnumWindows(EnumRobloxProc, reinterpret_cast<LPARAM>(&best));return best.hwnd; }
static DWORD GetWindowProcessIdSafe(HWND hwnd) { DWORD pid = 0;if (hwnd)GetWindowThreadProcessId(hwnd, &pid);return pid; }
static void RefreshRobloxPid() { g_robloxPid = GetWindowProcessIdSafe(g_robloxHwnd); }
static bool IsRobloxProcessWindow(HWND hwnd) { return hwnd && g_robloxPid != 0 && GetWindowProcessIdSafe(hwnd) == g_robloxPid; }
static void SetOverlayVisible(bool visible) { if (!g_overlayHwnd)return;if (g_overlayVisible == visible)return;g_overlayVisible = visible;ShowWindow(g_overlayHwnd, visible ? SW_SHOWNA : SW_HIDE); }
struct EffectSettings {
	float glossyFloorIntensity = 0.45f, glossyRoughness = 0.001f, glossyFresnelPower = 1.5f, glossySpecularGlint = 1.0f, glossyContrast = 1.0f;
	bool glossyEnabled = true, clearCoatMode = false, textProtectEnabled = true;
	int engineMode = 0, themeMode = 0, tonemapMode = 0, presetIdx = 0;
	float glossyTint[3] = { 1,1,1 }, specularTint[3] = { 1,1,1 }, outlineColor[3] = { 0,0,0 }, fogColor[3] = { 0.7f,0.8f,0.9f };
	float groundFadeEnd = 1.0f, ssaoIntensity = 1.0f, ssaoRadius = 2.0f;bool ssaoEnabled = false;
	float waveDistortionAmount = 0.0f, waveSpeed = 1.0f, waveScale = 45.0f, neonGlowIntensity = 0.0f, neonPinkBoost = 1.0f, neonBlueFog = 0.0f;
	float bloomIntensity = 0.0f, bloomThreshold = 0.85f, chromaticAberration = 0.0f, vignetteIntensity = 0.0f, vignetteSmoothness = 0.5f;
	float filmGrainAmount = 0.0f, sharpeningAmount = 0.0f, sharpeningRadius = 1.0f;
	float vibranceAmount = 0.0f, horizonY = 0.42f, skyGlowStrength = 0.0f, exposure = 0.0f, tempKelvin = 6500.0f;
	bool outlineEnabled = false;float outlineThickness = 1.5f;
	bool dofEnabled = false;float dofFocusDistance = 0.35f, dofRange = 0.2f, dofBlurStrength = 0.5f;
	bool fogEnabled = false;float fogDistance = 0.5f, fogDensity = 0.4f;
	bool splitScreenEnabled = false;float splitScreenPos = 0.5f;
	bool upscaleFSR = false, upscaleNAFNet = false, upscaleDLSS = false, upscaleTemporalSharpen = false;
	float upscaleStrength = 0.75f;int upscaleQuality = 1;
	bool fxaaEnabled = false;float fsrSharpness = 0.6f;
	float godRayIntensity = 0.0f, godRayDecay = 0.93f, godRayX = 0.5f, godRayY = 0.18f;
	float motionBlurAmount = 0.0f;
	bool crosshairEnabled = false;int crosshairStyle = 0;
	float crosshairSize = 9.0f, crosshairGap = 4.0f, crosshairThickness = 2.0f, crosshairOpacity = 1.0f;
	float crosshairColor[3] = { 0.15f,1.0f,0.25f };
	float scanlineIntensity = 0.0f, crtCurve = 0.0f, glitchAmount = 0.0f;
	float zoomBlur = 0.0f, tiltShift = 0.0f, panini = 0.0f;
	float sunAngle = 45.0f, ambientLight = 0.2f;
	float rainDrops = 0.0f, windDistortion = 0.0f, snowAmount = 0.0f;
	float nightVision = 0.0f, thermalVision = 0.0f;
	float shadowTint[3] = { 0.2f,0.2f,0.3f }, contactShadow = 0.0f;
	float matRoughness = 1.0f, matMetalness = 0.0f;
	float bloomSpectrum = 0.0f, lensDirt = 0.0f;
	float uiScale = 1.0f, fpsLimit = 0.0f;
	float customFx[40] = { 0 };
	bool upscaleSD = false;
};
struct SavedSlot { bool valid = false;EffectSettings fx; };
static SavedSlot g_userSlots[4];
static void ApplyPreset(EffectSettings& fx, int p) {
	fx = EffectSettings{};fx.presetIdx = p;
	switch (p) {
	case 0: fx.glossyEnabled = true;fx.glossyFloorIntensity = 1.0f;fx.glossyRoughness = 0.0020f;fx.bloomIntensity = 0.30f;fx.vibranceAmount = 0.18f;fx.tonemapMode = 1;break;
	case 1: fx.fxaaEnabled = true;fx.upscaleFSR = true;fx.fsrSharpness = 0.55f;fx.sharpeningAmount = 0.25f;break;
	case 2: fx.themeMode = 3;fx.neonGlowIntensity = 1.2f;fx.neonPinkBoost = 1.4f;fx.neonBlueFog = 0.22f;fx.glossyFloorIntensity = 1.1f;fx.bloomIntensity = 0.65f;fx.godRayIntensity = 0.25f;break;
	case 3: fx.glossyEnabled = true;fx.glossyFloorIntensity = 0.9f;fx.glossyRoughness = 0.010f;fx.groundFadeEnd = 0.72f;fx.vignetteIntensity = 0.20f;break;
	case 4: fx.waveDistortionAmount = 0.55f;fx.waveSpeed = 1.6f;fx.waveScale = 36.0f;fx.rainDrops = 0.55f;fx.glossyEnabled = true;fx.glossyFloorIntensity = 1.25f;break;
	case 5: fx.outlineEnabled = true;fx.outlineThickness = 2.2f;fx.themeMode = 9;fx.vibranceAmount = 0.32f;break;
	case 6: fx.themeMode = 4;fx.tonemapMode = 1;fx.exposure = 0.15f;fx.skyGlowStrength = 0.18f;break;
	case 7: fx.themeMode = 13;fx.fogEnabled = true;fx.fogDistance = 0.45f;fx.fogDensity = 0.48f;fx.rainDrops = 0.35f;break;
	case 8: fx.themeMode = 19;fx.exposure = 0.10f;fx.bloomIntensity = 0.45f;fx.godRayIntensity = 0.18f;fx.skyGlowStrength = 0.35f;break;
	default: break;
	}
}
struct AppConfig { EffectSettings fx;bool autoTrackWindow = true; };
static void SaveConfig(const AppConfig& cfg) { std::ofstream f("studsettings.cfg", std::ios::binary);if (!f.is_open())return;f.write(reinterpret_cast<const char*>(&cfg), sizeof(AppConfig)); }
static void LoadConfig(AppConfig& cfg) { std::ifstream f("studsettings.cfg", std::ios::binary);if (!f.is_open())return;f.read(reinterpret_cast<char*>(&cfg), sizeof(AppConfig)); }
static std::string TrimJsonValue(std::string s) { while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r' || s.front() == '\n'))s.erase(s.begin());while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n'))s.pop_back();return s; }
static bool JsonFindValue(const std::string& json, const std::string& key, std::string& out) {
	size_t pos = json.find("\"" + key + "\"");if (pos == std::string::npos)pos = json.find(key);if (pos == std::string::npos)return false;
	pos = json.find(':', pos);if (pos == std::string::npos)return false;size_t start = pos + 1;
	while (start < json.size() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\r' || json[start] == '\n'))start++;if (start >= json.size())return false;
	if (json[start] == '[') { int depth = 0;size_t end = start;for (;end < json.size();++end) { if (json[end] == '[')depth++;else if (json[end] == ']') { depth--;if (depth == 0) { ++end;break; } } }out = TrimJsonValue(json.substr(start, end - start));return !out.empty(); }
	size_t end = start;while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n' && json[end] != '\r')end++;out = TrimJsonValue(json.substr(start, end - start));return !out.empty();
}
static bool JsonReadFloat(const std::string& json, const std::string& key, float& v) { std::string raw;if (!JsonFindValue(json, key, raw))return false;try { v = std::stof(raw);return true; } catch (...) { return false; } }
static bool JsonReadInt(const std::string& json, const std::string& key, int& v) { std::string raw;if (!JsonFindValue(json, key, raw))return false;try { v = std::stoi(raw);return true; } catch (...) { return false; } }
static bool JsonReadBool(const std::string& json, const std::string& key, bool& v) { std::string raw;if (!JsonFindValue(json, key, raw))return false;std::transform(raw.begin(), raw.end(), raw.begin(), [](unsigned char ch) {return static_cast<char>(std::tolower(ch));});if (raw == "true" || raw == "1") { v = true;return true; }if (raw == "false" || raw == "0") { v = false;return true; }return false; }
static bool JsonReadFloat3(const std::string& json, const std::string& key, float outv[3]) { std::string raw;if (!JsonFindValue(json, key, raw))return false;if (raw.size() < 5 || raw.front() != '[' || raw.back() != ']')return false;raw = raw.substr(1, raw.size() - 2);std::replace(raw.begin(), raw.end(), ',', ' ');std::istringstream iss(raw);return static_cast<bool>(iss >> outv[0] >> outv[1] >> outv[2]); }
static void JsonWriteFloat3(std::ostringstream& oss, const char* key, const float v[3], bool comma = true) { oss << " \"" << key << "\":[" << v[0] << "," << v[1] << "," << v[2] << "]";if (comma)oss << ",";oss << "\n"; }
static std::string EffectSettingsToJson(const EffectSettings& fx) {
	std::ostringstream oss;oss << "{\n";
	oss << " \"glossyFloorIntensity\": " << fx.glossyFloorIntensity << ",\n";
	oss << " \"glossyRoughness\": " << fx.glossyRoughness << ",\n";
	oss << " \"glossyFresnelPower\": " << fx.glossyFresnelPower << ",\n";
	oss << " \"glossySpecularGlint\": " << fx.glossySpecularGlint << ",\n";
	oss << " \"glossyContrast\": " << fx.glossyContrast << ",\n";
	oss << " \"glossyEnabled\": " << (fx.glossyEnabled ? "true" : "false") << ",\n";
	oss << " \"clearCoatMode\": " << (fx.clearCoatMode ? "true" : "false") << ",\n";
	oss << " \"textProtectEnabled\": " << (fx.textProtectEnabled ? "true" : "false") << ",\n";
	oss << " \"engineMode\": " << fx.engineMode << ",\n";
	oss << " \"themeMode\": " << fx.themeMode << ",\n";
	oss << " \"tonemapMode\": " << fx.tonemapMode << ",\n";
	oss << " \"presetIdx\": " << fx.presetIdx << ",\n";
	JsonWriteFloat3(oss, "glossyTint", fx.glossyTint);
	JsonWriteFloat3(oss, "specularTint", fx.specularTint);
	JsonWriteFloat3(oss, "outlineColor", fx.outlineColor);
	JsonWriteFloat3(oss, "fogColor", fx.fogColor);
	oss << " \"groundFadeEnd\": " << fx.groundFadeEnd << ",\n";
	oss << " \"ssaoIntensity\": " << fx.ssaoIntensity << ",\n";
	oss << " \"ssaoRadius\": " << fx.ssaoRadius << ",\n";
	oss << " \"ssaoEnabled\": " << (fx.ssaoEnabled ? "true" : "false") << ",\n";
	oss << " \"waveDistortionAmount\": " << fx.waveDistortionAmount << ",\n";
	oss << " \"waveSpeed\": " << fx.waveSpeed << ",\n";
	oss << " \"waveScale\": " << fx.waveScale << ",\n";
	oss << " \"neonGlowIntensity\": " << fx.neonGlowIntensity << ",\n";
	oss << " \"neonPinkBoost\": " << fx.neonPinkBoost << ",\n";
	oss << " \"neonBlueFog\": " << fx.neonBlueFog << ",\n";
	oss << " \"bloomIntensity\": " << fx.bloomIntensity << ",\n";
	oss << " \"bloomThreshold\": " << fx.bloomThreshold << ",\n";
	oss << " \"chromaticAberration\": " << fx.chromaticAberration << ",\n";
	oss << " \"vignetteIntensity\": " << fx.vignetteIntensity << ",\n";
	oss << " \"vignetteSmoothness\": " << fx.vignetteSmoothness << ",\n";
	oss << " \"filmGrainAmount\": " << fx.filmGrainAmount << ",\n";
	oss << " \"sharpeningAmount\": " << fx.sharpeningAmount << ",\n";
	oss << " \"sharpeningRadius\": " << fx.sharpeningRadius << ",\n";
	oss << " \"vibranceAmount\": " << fx.vibranceAmount << ",\n";
	oss << " \"horizonY\": " << fx.horizonY << ",\n";
	oss << " \"skyGlowStrength\": " << fx.skyGlowStrength << ",\n";
	oss << " \"exposure\": " << fx.exposure << ",\n";
	oss << " \"tempKelvin\": " << fx.tempKelvin << ",\n";
	oss << " \"outlineEnabled\": " << (fx.outlineEnabled ? "true" : "false") << ",\n";
	oss << " \"outlineThickness\": " << fx.outlineThickness << ",\n";
	oss << " \"dofEnabled\": " << (fx.dofEnabled ? "true" : "false") << ",\n";
	oss << " \"dofFocusDistance\": " << fx.dofFocusDistance << ",\n";
	oss << " \"dofRange\": " << fx.dofRange << ",\n";
	oss << " \"dofBlurStrength\": " << fx.dofBlurStrength << ",\n";
	oss << " \"fogEnabled\": " << (fx.fogEnabled ? "true" : "false") << ",\n";
	oss << " \"fogDistance\": " << fx.fogDistance << ",\n";
	oss << " \"fogDensity\": " << fx.fogDensity << ",\n";
	oss << " \"splitScreenEnabled\": " << (fx.splitScreenEnabled ? "true" : "false") << ",\n";
	oss << " \"splitScreenPos\": " << fx.splitScreenPos << ",\n";
	oss << " \"upscaleFSR\": " << (fx.upscaleFSR ? "true" : "false") << ",\n";
	oss << " \"upscaleNAFNet\": " << (fx.upscaleNAFNet ? "true" : "false") << ",\n";
	oss << " \"upscaleDLSS\": " << (fx.upscaleDLSS ? "true" : "false") << ",\n";
	oss << " \"upscaleTemporalSharpen\": " << (fx.upscaleTemporalSharpen ? "true" : "false") << ",\n";
	oss << " \"upscaleStrength\": " << fx.upscaleStrength << ",\n";
	oss << " \"upscaleQuality\": " << fx.upscaleQuality << ",\n";
	oss << " \"fxaaEnabled\": " << (fx.fxaaEnabled ? "true" : "false") << ",\n";
	oss << " \"fsrSharpness\": " << fx.fsrSharpness << ",\n";
	oss << " \"godRayIntensity\": " << fx.godRayIntensity << ",\n";
	oss << " \"godRayDecay\": " << fx.godRayDecay << ",\n";
	oss << " \"godRayX\": " << fx.godRayX << ",\n";
	oss << " \"godRayY\": " << fx.godRayY << ",\n";
	oss << " \"motionBlurAmount\": " << fx.motionBlurAmount << ",\n";
	oss << " \"crosshairEnabled\": " << (fx.crosshairEnabled ? "true" : "false") << ",\n";
	oss << " \"crosshairStyle\": " << fx.crosshairStyle << ",\n";
	oss << " \"crosshairSize\": " << fx.crosshairSize << ",\n";
	oss << " \"crosshairGap\": " << fx.crosshairGap << ",\n";
	oss << " \"crosshairThickness\": " << fx.crosshairThickness << ",\n";
	oss << " \"crosshairOpacity\": " << fx.crosshairOpacity << ",\n";
	JsonWriteFloat3(oss, "crosshairColor", fx.crosshairColor);
	oss << " \"scanlineIntensity\": " << fx.scanlineIntensity << ",\n";
	oss << " \"crtCurve\": " << fx.crtCurve << ",\n";
	oss << " \"glitchAmount\": " << fx.glitchAmount << ",\n";
	oss << " \"zoomBlur\": " << fx.zoomBlur << ",\n";
	oss << " \"tiltShift\": " << fx.tiltShift << ",\n";
	oss << " \"panini\": " << fx.panini << ",\n";
	oss << " \"sunAngle\": " << fx.sunAngle << ",\n";
	oss << " \"ambientLight\": " << fx.ambientLight << ",\n";
	oss << " \"rainDrops\": " << fx.rainDrops << ",\n";
	oss << " \"windDistortion\": " << fx.windDistortion << ",\n";
	oss << " \"snowAmount\": " << fx.snowAmount << ",\n";
	oss << " \"nightVision\": " << fx.nightVision << ",\n";
	oss << " \"thermalVision\": " << fx.thermalVision << ",\n";
	JsonWriteFloat3(oss, "shadowTint", fx.shadowTint);
	oss << " \"contactShadow\": " << fx.contactShadow << ",\n";
	oss << " \"matRoughness\": " << fx.matRoughness << ",\n";
	oss << " \"matMetalness\": " << fx.matMetalness << ",\n";
	oss << " \"bloomSpectrum\": " << fx.bloomSpectrum << ",\n";
	oss << " \"lensDirt\": " << fx.lensDirt << ",\n";
	oss << " \"uiScale\": " << fx.uiScale << ",\n";
	oss << " \"fpsLimit\": " << fx.fpsLimit << ",\n";
	oss << " \"upscaleSD\": " << (fx.upscaleSD ? "true" : "false") << ",\n";
	oss << " \"customFx\":[";for (int i = 0;i < 40;++i) { if (i)oss << ",";oss << fx.customFx[i]; }oss << "]\n";oss << "}";return oss.str();
}
static bool JsonToEffectSettings(const std::string& json, EffectSettings& fx) {
	if (json.empty())return false;
	JsonReadFloat(json, "glossyFloorIntensity", fx.glossyFloorIntensity);JsonReadFloat(json, "glossyRoughness", fx.glossyRoughness);
	JsonReadFloat(json, "glossyFresnelPower", fx.glossyFresnelPower);JsonReadFloat(json, "glossySpecularGlint", fx.glossySpecularGlint);
	JsonReadFloat(json, "glossyContrast", fx.glossyContrast);JsonReadBool(json, "glossyEnabled", fx.glossyEnabled);
	JsonReadBool(json, "clearCoatMode", fx.clearCoatMode);JsonReadBool(json, "textProtectEnabled", fx.textProtectEnabled);
	JsonReadInt(json, "engineMode", fx.engineMode);JsonReadInt(json, "themeMode", fx.themeMode);
	JsonReadInt(json, "tonemapMode", fx.tonemapMode);JsonReadInt(json, "presetIdx", fx.presetIdx);
	JsonReadFloat3(json, "glossyTint", fx.glossyTint);JsonReadFloat3(json, "specularTint", fx.specularTint);
	JsonReadFloat3(json, "outlineColor", fx.outlineColor);JsonReadFloat3(json, "fogColor", fx.fogColor);
	JsonReadFloat(json, "groundFadeEnd", fx.groundFadeEnd);JsonReadFloat(json, "ssaoIntensity", fx.ssaoIntensity);
	JsonReadFloat(json, "ssaoRadius", fx.ssaoRadius);JsonReadBool(json, "ssaoEnabled", fx.ssaoEnabled);
	JsonReadFloat(json, "waveDistortionAmount", fx.waveDistortionAmount);JsonReadFloat(json, "waveSpeed", fx.waveSpeed);
	JsonReadFloat(json, "waveScale", fx.waveScale);JsonReadFloat(json, "neonGlowIntensity", fx.neonGlowIntensity);
	JsonReadFloat(json, "neonPinkBoost", fx.neonPinkBoost);JsonReadFloat(json, "neonBlueFog", fx.neonBlueFog);
	JsonReadFloat(json, "bloomIntensity", fx.bloomIntensity);JsonReadFloat(json, "bloomThreshold", fx.bloomThreshold);
	JsonReadFloat(json, "chromaticAberration", fx.chromaticAberration);JsonReadFloat(json, "vignetteIntensity", fx.vignetteIntensity);
	JsonReadFloat(json, "vignetteSmoothness", fx.vignetteSmoothness);JsonReadFloat(json, "filmGrainAmount", fx.filmGrainAmount);
	JsonReadFloat(json, "sharpeningAmount", fx.sharpeningAmount);JsonReadFloat(json, "sharpeningRadius", fx.sharpeningRadius);
	JsonReadFloat(json, "vibranceAmount", fx.vibranceAmount);JsonReadFloat(json, "horizonY", fx.horizonY);
	JsonReadFloat(json, "skyGlowStrength", fx.skyGlowStrength);JsonReadFloat(json, "exposure", fx.exposure);
	JsonReadFloat(json, "tempKelvin", fx.tempKelvin);JsonReadBool(json, "outlineEnabled", fx.outlineEnabled);
	JsonReadFloat(json, "outlineThickness", fx.outlineThickness);JsonReadBool(json, "dofEnabled", fx.dofEnabled);
	JsonReadFloat(json, "dofFocusDistance", fx.dofFocusDistance);JsonReadFloat(json, "dofRange", fx.dofRange);
	JsonReadFloat(json, "dofBlurStrength", fx.dofBlurStrength);JsonReadBool(json, "fogEnabled", fx.fogEnabled);
	JsonReadFloat(json, "fogDistance", fx.fogDistance);JsonReadFloat(json, "fogDensity", fx.fogDensity);
	JsonReadBool(json, "splitScreenEnabled", fx.splitScreenEnabled);JsonReadFloat(json, "splitScreenPos", fx.splitScreenPos);
	JsonReadBool(json, "upscaleFSR", fx.upscaleFSR);JsonReadBool(json, "upscaleNAFNet", fx.upscaleNAFNet);
	JsonReadBool(json, "upscaleDLSS", fx.upscaleDLSS);JsonReadBool(json, "upscaleTemporalSharpen", fx.upscaleTemporalSharpen);
	JsonReadFloat(json, "upscaleStrength", fx.upscaleStrength);JsonReadInt(json, "upscaleQuality", fx.upscaleQuality);
	JsonReadBool(json, "fxaaEnabled", fx.fxaaEnabled);JsonReadFloat(json, "fsrSharpness", fx.fsrSharpness);
	JsonReadFloat(json, "godRayIntensity", fx.godRayIntensity);JsonReadFloat(json, "godRayDecay", fx.godRayDecay);
	JsonReadFloat(json, "godRayX", fx.godRayX);JsonReadFloat(json, "godRayY", fx.godRayY);
	JsonReadFloat(json, "motionBlurAmount", fx.motionBlurAmount);JsonReadBool(json, "crosshairEnabled", fx.crosshairEnabled);
	JsonReadInt(json, "crosshairStyle", fx.crosshairStyle);JsonReadFloat(json, "crosshairSize", fx.crosshairSize);
	JsonReadFloat(json, "crosshairGap", fx.crosshairGap);JsonReadFloat(json, "crosshairThickness", fx.crosshairThickness);
	JsonReadFloat(json, "crosshairOpacity", fx.crosshairOpacity);JsonReadFloat3(json, "crosshairColor", fx.crosshairColor);
	JsonReadFloat(json, "scanlineIntensity", fx.scanlineIntensity);JsonReadFloat(json, "crtCurve", fx.crtCurve);
	JsonReadFloat(json, "glitchAmount", fx.glitchAmount);JsonReadFloat(json, "zoomBlur", fx.zoomBlur);
	JsonReadFloat(json, "tiltShift", fx.tiltShift);JsonReadFloat(json, "panini", fx.panini);
	JsonReadFloat(json, "sunAngle", fx.sunAngle);JsonReadFloat(json, "ambientLight", fx.ambientLight);
	JsonReadFloat(json, "rainDrops", fx.rainDrops);JsonReadFloat(json, "windDistortion", fx.windDistortion);
	JsonReadFloat(json, "snowAmount", fx.snowAmount);JsonReadFloat(json, "nightVision", fx.nightVision);
	JsonReadFloat(json, "thermalVision", fx.thermalVision);JsonReadFloat3(json, "shadowTint", fx.shadowTint);
	JsonReadFloat(json, "contactShadow", fx.contactShadow);JsonReadFloat(json, "matRoughness", fx.matRoughness);
	JsonReadFloat(json, "matMetalness", fx.matMetalness);JsonReadFloat(json, "bloomSpectrum", fx.bloomSpectrum);
	JsonReadFloat(json, "lensDirt", fx.lensDirt);JsonReadFloat(json, "uiScale", fx.uiScale);
	JsonReadFloat(json, "fpsLimit", fx.fpsLimit);JsonReadBool(json, "upscaleSD", fx.upscaleSD);
	{ std::string raw;if (JsonFindValue(json, "customFx", raw) && raw.size() >= 2 && raw.front() == '[' && raw.back() == ']') { raw = raw.substr(1, raw.size() - 2);std::replace(raw.begin(), raw.end(), ',', ' ');std::istringstream iss(raw);for (int i = 0;i < 40;++i) { if (!(iss >> fx.customFx[i]))break; } } }
	return true;
}
static void SaveConfigToFile(const AppConfig& cfg) {
	OPENFILENAMEW ofn = {};wchar_t szFile[MAX_PATH] = L"studsettings.json";ofn.lStructSize = sizeof(ofn);ofn.lpstrFile = szFile;ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = nullptr;
	ofn.Flags = OFN_OVERWRITEPROMPT;if (!GetSaveFileNameW(&ofn))return;
	std::ofstream f{ std::filesystem::path(szFile) };if (f.is_open())f << EffectSettingsToJson(cfg.fx);
}
static void LoadConfigFromFile(AppConfig& cfg) {
	OPENFILENAMEW ofn = {};wchar_t szFile[MAX_PATH] = L"studsettings.json";ofn.lStructSize = sizeof(ofn);ofn.lpstrFile = szFile;ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = nullptr;
	ofn.Flags = OFN_FILEMUSTEXIST;if (!GetOpenFileNameW(&ofn))return;
	std::ifstream f{ std::filesystem::path(szFile) };if (!f.is_open())return;std::stringstream ss;ss << f.rdbuf();JsonToEffectSettings(ss.str(), cfg.fx);
}
struct PerfSample { double overlayFps = 0, aiPassMs = 0; };
class PerfTracker {
public:
	void PushAiMs(double ms) { m_ai.push_back(ms);if (m_ai.size() > 90)m_ai.pop_front(); }
	void PushTick() { auto now = std::chrono::steady_clock::now();if (m_last.time_since_epoch().count()) { m_ft.push_back(std::chrono::duration<double, std::milli>(now - m_last).count());if (m_ft.size() > 90)m_ft.pop_front(); }m_last = now; }
	PerfSample Snap() {
		PerfSample s;
		s.aiPassMs = 0.0;
		double ft = m_ft.empty() ? 0.0 : std::accumulate(m_ft.begin(), m_ft.end(), 0.0) / static_cast<double>(m_ft.size());
		double raw = ft > 0.0 ? 1000.0 / ft : 0.0;
		if (raw > 1000.0)raw = 1000.0;
		if (m_lastFps < 1.0)m_lastFps = raw;
		else m_lastFps = m_lastFps * 0.8 + raw * 0.2;
		s.overlayFps = m_lastFps;
		return s;
	}
private: std::deque<double> m_ai, m_ft;std::chrono::steady_clock::time_point m_last{};double m_lastFps = 0.0;
};
#ifndef WINHTTP_QUERY_CONTENT_RANGE
#define WINHTTP_QUERY_CONTENT_RANGE 44
#endif
#ifndef WINHTTP_OPTION_ENABLE_HTTP_PROTOCOL
#define WINHTTP_OPTION_ENABLE_HTTP_PROTOCOL 133
#define WINHTTP_PROTOCOL_FLAG_HTTP2 0x1
#endif
static void EnableHttp2(HINTERNET req) {
	DWORD proto = WINHTTP_PROTOCOL_FLAG_HTTP2;
	WinHttpSetOption(req, WINHTTP_OPTION_ENABLE_HTTP_PROTOCOL, &proto, sizeof(proto));
}
static bool DownloadUrlToFile(const std::wstring& url, const std::wstring& dest, std::atomic<double>* prog, std::string* errOut) {
	std::wstring cur = url;int redir = 0;
	while (redir < 10) {
		URL_COMPONENTS uc{};uc.dwStructSize = sizeof(uc);std::vector<wchar_t> host(256, 0), path(8192, 0);
		uc.lpszHostName = host.data();uc.dwHostNameLength = static_cast<DWORD>(host.size());uc.lpszUrlPath = path.data();uc.dwUrlPathLength = static_cast<DWORD>(path.size());
		if (!WinHttpCrackUrl(cur.c_str(), static_cast<DWORD>(cur.size()), 0, &uc)) { if (errOut)*errOut = "URL parse failed";return false; }
		HINTERNET ses = WinHttpOpen(L"Mozilla/5.0(Windows NT 10.0;Win64;x64)AppleWebKit/537.36(KHTML,like Gecko)Chrome/126.0.0.0 Safari/537.36", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);if (!ses)return false;
		HINTERNET con = WinHttpConnect(ses, uc.lpszHostName, uc.nPort, 0);if (!con) { WinHttpCloseHandle(ses);return false; }
		DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
		HINTERNET req = WinHttpOpenRequest(con, L"GET", uc.lpszUrlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
		if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);return false; }
		DWORD pol = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;WinHttpSetOption(req, WINHTTP_OPTION_REDIRECT_POLICY, &pol, sizeof(pol));
		EnableHttp2(req);
		if (!WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) || !WinHttpReceiveResponse(req, nullptr)) { WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);return false; }
		DWORD status = 0, sz = sizeof(status);WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
		if (status == 301 || status == 302 || status == 303 || status == 307 || status == 308) {
			std::vector<wchar_t> loc(8192, 0);DWORD locSz = static_cast<DWORD>(loc.size() * sizeof(wchar_t));
			if (WinHttpQueryHeaders(req, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX, loc.data(), &locSz, WINHTTP_NO_HEADER_INDEX)) {
				cur = loc.data();WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);++redir;continue;
			}
		}
		if (status != 200) { WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);if (errOut)*errOut = "HTTP " + std::to_string(status);return false; }
		DWORD clen = 0;sz = sizeof(clen);WinHttpQueryHeaders(req, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &clen, &sz, WINHTTP_NO_HEADER_INDEX);
		HANDLE hf = CreateFileW(dest.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hf == INVALID_HANDLE_VALUE) { WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);if (errOut)*errOut = "cannot create local file";return false; }
		std::vector<char> buf(1024 * 1024);DWORD total = 0;
		for (;;) {
			DWORD avail = 0;if (!WinHttpQueryDataAvailable(req, &avail) || avail == 0)break;DWORD toRead = std::min<DWORD>(avail, static_cast<DWORD>(buf.size()));DWORD got = 0;
			if (!WinHttpReadData(req, buf.data(), toRead, &got) || got == 0)break;DWORD wr = 0;if (!WriteFile(hf, buf.data(), got, &wr, nullptr))break;total += got;
			if (clen && prog)prog->store(static_cast<double>(total) / static_cast<double>(clen));
		}
		CloseHandle(hf);WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);if (prog)prog->store(1.0);return true;
	}
	return false;
}
static int64_t QueryRemoteSize(const std::wstring& url) {
	URL_COMPONENTS uc{};uc.dwStructSize = sizeof(uc);std::vector<wchar_t> host(256, 0), path(8192, 0);
	uc.lpszHostName = host.data();uc.dwHostNameLength = static_cast<DWORD>(host.size());uc.lpszUrlPath = path.data();uc.dwUrlPathLength = static_cast<DWORD>(path.size());
	if (!WinHttpCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &uc))return -1;
	HINTERNET ses = WinHttpOpen(L"Mozilla/5.0(Windows NT 10.0;Win64;x64)AppleWebKit/537.36(KHTML,like Gecko)Chrome/126.0.0.0 Safari/537.36", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!ses)return -1;
	HINTERNET con = WinHttpConnect(ses, uc.lpszHostName, uc.nPort, 0);
	if (!con) { WinHttpCloseHandle(ses);return -1; }
	DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
	HINTERNET req = WinHttpOpenRequest(con, L"GET", uc.lpszUrlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
	if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);return -1; }
	EnableHttp2(req);
	int64_t sizeOut = -1;
	LPCWSTR hdrs = L"Range: bytes=0-0\r\n";
	if (WinHttpSendRequest(req, hdrs, -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(req, nullptr)) {
		DWORD status = 0, sz = sizeof(status);
		if (WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX) && status == 206) {
			wchar_t cr[64] = {};DWORD crsz = sizeof(cr);
			if (WinHttpQueryHeaders(req, WINHTTP_QUERY_CONTENT_RANGE, WINHTTP_HEADER_NAME_BY_INDEX, cr, &crsz, WINHTTP_NO_HEADER_INDEX)) {
				for (int k = 63;k > 0;--k)if (cr[k] == L'/') { sizeOut = wcstoll(cr + k + 1, nullptr, 10);break; }
			}
		}
	}
	WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
	return sizeOut;
}
static bool DownloadRange(const std::wstring& url, const std::wstring& partPath, int64_t start, int64_t end,
	std::atomic<int64_t>* doneBytes, const std::atomic<bool>* cancel, std::string* errOut) {
	for (int attempt = 0;attempt < 3;++attempt) {
		if (cancel && cancel->load())return false;
		URL_COMPONENTS uc{};uc.dwStructSize = sizeof(uc);std::vector<wchar_t> host(256, 0), path(8192, 0);
		uc.lpszHostName = host.data();uc.dwHostNameLength = static_cast<DWORD>(host.size());uc.lpszUrlPath = path.data();uc.dwUrlPathLength = static_cast<DWORD>(path.size());
		if (!WinHttpCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &uc)) { if (errOut)*errOut = "URL parse failed";return false; }
		HINTERNET ses = WinHttpOpen(L"Mozilla/5.0(Windows NT 10.0;Win64;x64)AppleWebKit/537.36(KHTML,like Gecko)Chrome/126.0.0.0 Safari/537.36", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!ses) { if (errOut)*errOut = "network init failed";return false; }
		HINTERNET con = WinHttpConnect(ses, uc.lpszHostName, uc.nPort, 0);
		if (!con) { WinHttpCloseHandle(ses);if (errOut)*errOut = "connect failed";return false; }
		DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
		HINTERNET req = WinHttpOpenRequest(con, L"GET", uc.lpszUrlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
		if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);if (errOut)*errOut = "request failed";return false; }
		DWORD pol = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;WinHttpSetOption(req, WINHTTP_OPTION_REDIRECT_POLICY, &pol, sizeof(pol));
		EnableHttp2(req);
		DWORD to = 15000;WinHttpSetOption(req, WINHTTP_OPTION_CONNECT_TIMEOUT, &to, sizeof(to));
		to = 30000;WinHttpSetOption(req, WINHTTP_OPTION_RECEIVE_TIMEOUT, &to, sizeof(to));
		to = 30000;WinHttpSetOption(req, WINHTTP_OPTION_SEND_TIMEOUT, &to, sizeof(to));
		std::wstring rangeHdr = L"Range: bytes=" + std::to_wstring(start) + L"-" + std::to_wstring(end) + L"\r\n";
		if (!WinHttpSendRequest(req, rangeHdr.c_str(), -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) || !WinHttpReceiveResponse(req, nullptr)) {
			WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);continue;
		}
		DWORD status = 0, sz = sizeof(status);
		WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
		if (status != 206 && status != 200) { WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);continue; }
		HANDLE hf = CreateFileW(partPath.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hf == INVALID_HANDLE_VALUE) { WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);if (errOut)*errOut = "cannot open temp file";return false; }
		LARGE_INTEGER off{};off.QuadPart = start;
		if (!SetFilePointerEx(hf, off, nullptr, FILE_BEGIN)) { CloseHandle(hf);WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);continue; }
		std::vector<char> buf(1024 * 1024);
		int64_t wrote = 0;
		int64_t need = end - start + 1;
		bool ok = true;
		if (status == 200 && start > 0) {
			int64_t toSkip = start;
			while (toSkip > 0) {
				DWORD avail = 0;
				if (!WinHttpQueryDataAvailable(req, &avail) || avail == 0) { ok = false;break; }
				DWORD toRead = (DWORD)std::min<int64_t>(avail, std::min<int64_t>(toSkip, (int64_t)buf.size()));
				DWORD got = 0;
				if (!WinHttpReadData(req, buf.data(), toRead, &got) || got == 0) { ok = false;break; }
				toSkip -= got;
			}
		}
		while (ok && wrote < need) {
			if (cancel && cancel->load()) { ok = false;break; }
			DWORD avail = 0;
			if (!WinHttpQueryDataAvailable(req, &avail) || avail == 0)break;
			DWORD toRead = (DWORD)std::min<int64_t>(avail, std::min<int64_t>(need - wrote, (int64_t)buf.size()));
			DWORD got = 0;
			if (!WinHttpReadData(req, buf.data(), toRead, &got) || got == 0) { ok = false;break; }
			DWORD wr = 0;
			if (!WriteFile(hf, buf.data(), got, &wr, nullptr)) { ok = false;break; }
			wrote += got;
			if (doneBytes)doneBytes->fetch_add(got);
		}
		CloseHandle(hf);WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
		if (ok && wrote >= need)return true;
		if (doneBytes && wrote > 0)doneBytes->fetch_sub(wrote);
	}
	if (errOut && errOut->empty())*errOut = "download failed after retries";
	return false;
}
static bool DownloadFileSmart(const std::wstring& url, const std::wstring& dest, int64_t total,
	std::atomic<int64_t>* doneBytes, const std::atomic<bool>* cancel, std::string* errOut) {
	if (total <= 0) { if (errOut)*errOut = "unknown file size";return false; }
	int64_t rangeSize = 32 * 1024 * 1024;
	if (total < 4 * 1024 * 1024)rangeSize = total;
	int nRanges = (int)((total + rangeSize - 1) / rangeSize);
	int workers = 1;
	if (total >= 256 * 1024 * 1024)workers = 8;
	else if (total >= 32 * 1024 * 1024)workers = 4;
	else if (total >= 4 * 1024 * 1024)workers = 2;
	std::wstring part = dest + L".part";
	{
		HANDLE pf = CreateFileW(part.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (pf == INVALID_HANDLE_VALUE) { if (errOut)*errOut = "cannot create temp file";return false; }
		LARGE_INTEGER sz{};sz.QuadPart = total;
		SetFilePointerEx(pf, sz, nullptr, FILE_BEGIN);
		SetEndOfFile(pf);
		CloseHandle(pf);
	}
	std::atomic<int> nextRange{ 0 };
	std::mutex failMtx;
	std::vector<int> failedRanges;
	auto work = [&] {
		for (;;) {
			if (cancel && cancel->load())return;
			int idx = nextRange.fetch_add(1);
			if (idx >= nRanges)return;
			int64_t s = (int64_t)idx * rangeSize;
			int64_t e = std::min(total - 1, s + rangeSize - 1);
			bool ok = false;
			for (int attempt = 0;attempt < 3 && !ok;++attempt) {
				if (cancel && cancel->load())return;
				ok = DownloadRange(url, part, s, e, doneBytes, cancel, nullptr);
			}
			if (!ok) {
				std::lock_guard<std::mutex> lk(failMtx);
				failedRanges.push_back(idx);
			}
		}
		};
	std::vector<std::thread> threads;
	for (int i = 0;i < workers;++i)threads.emplace_back(work);
	for (auto& t : threads)t.join();
	for (int idx : failedRanges) {
		if (cancel && cancel->load())break;
		int64_t s = (int64_t)idx * rangeSize;
		int64_t e = std::min(total - 1, s + rangeSize - 1);
		bool ok = false;
		for (int attempt = 0;attempt < 3 && !ok;++attempt)
			ok = DownloadRange(url, part, s, e, doneBytes, cancel, nullptr);
	}
	if (cancel && cancel->load()) { DeleteFileW(part.c_str());return false; }
	int64_t got = 0;
	if (doneBytes)got = doneBytes->load();
	if (got < total) {
		if (errOut)*errOut = "incomplete download after retries";
		DeleteFileW(part.c_str());
		return false;
	}
	DeleteFileW(dest.c_str());
	if (!MoveFileW(part.c_str(), dest.c_str())) { if (errOut)*errOut = "finalize failed";DeleteFileW(part.c_str());return false; }
	return true;
}
static bool ValidateModelFile(const std::wstring& p, int64_t minBytes) {
	HANDLE hf = CreateFileW(p.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hf == INVALID_HANDLE_VALUE)return false;LARGE_INTEGER sz{};GetFileSizeEx(hf, &sz);if (sz.QuadPart < minBytes) { CloseHandle(hf);return false; }
	uint8_t hdr[16] = {};DWORD got = 0;if (!ReadFile(hf, hdr, 16, &got, nullptr)) { CloseHandle(hf);return false; }CloseHandle(hf);
	if (got >= 1 && hdr[0] == '<')return false;return got >= 8;
}
static bool EndsWithNoCase(const std::wstring& s, const std::wstring& suffix) { if (suffix.size() > s.size())return false;for (size_t i = 0;i < suffix.size();++i) { wchar_t a = static_cast<wchar_t>(towlower(s[s.size() - suffix.size() + i]));wchar_t b = static_cast<wchar_t>(towlower(suffix[i]));if (a != b)return false; }return true; }
static std::wstring GetParentDir(const std::wstring& p) { size_t slash = p.find_last_of(L"\\/");return slash == std::wstring::npos ? L"." : p.substr(0, slash); }
static std::wstring GetRootExtractDirForModelPath(const std::wstring& modelPath) { size_t slash = modelPath.find_first_of(L"\\/");return slash == std::wstring::npos ? GetParentDir(modelPath) : modelPath.substr(0, slash); }
static bool RunHiddenProcess(const std::wstring& cmdLine) {
	STARTUPINFOW si{};PROCESS_INFORMATION pi{};si.cb = sizeof(si);si.dwFlags = STARTF_USESHOWWINDOW;si.wShowWindow = SW_HIDE;
	std::vector<wchar_t> buf(cmdLine.begin(), cmdLine.end());buf.push_back(0);
	if (!CreateProcessW(nullptr, buf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))return false;
	WaitForSingleObject(pi.hProcess, INFINITE);DWORD code = 1;GetExitCodeProcess(pi.hProcess, &code);CloseHandle(pi.hThread);CloseHandle(pi.hProcess);return code == 0;
}
static bool ExtractZipToDirectory(const std::wstring& zipPath, const std::wstring& destDir) {
	std::error_code ec;std::filesystem::create_directories(destDir, ec);
	std::wstring ps = L"powershell -NoProfile -ExecutionPolicy Bypass -Command \"Expand-Archive -LiteralPath '" + zipPath + L"' -DestinationPath '" + destDir + L"' -Force\"";
	if (RunHiddenProcess(ps))return true;std::wstring tar = L"tar -xf \"" + zipPath + L"\" -C \"" + destDir + L"\"";return RunHiddenProcess(tar);
}
enum class ProvState { Checking, Downloading, Verifying, Ready, Failed };
class GenericProvisioner {
public:
	void Begin(std::vector<std::wstring> urls, std::wstring path, int64_t minBytes) {
		bool expected = false;if (!m_started.compare_exchange_strong(expected, true))return;
		m_urls = std::move(urls);m_path = std::move(path);m_minBytes = minBytes;m_state = ProvState::Checking;m_prog.store(0.0);
		m_thread = std::thread([this] {SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);Work();});
	}
	void Retry() { if (m_thread.joinable())m_thread.join(); { std::lock_guard<std::mutex> lk(m_mtx);m_err.clear(); }m_started.store(false);m_prog.store(0.0);Begin(m_urls, m_path, m_minBytes); }
	void Reset() { if (m_thread.joinable())m_thread.join();m_started.store(false);m_state.store(ProvState::Checking);m_prog.store(0.0);std::lock_guard<std::mutex> lk(m_mtx);m_err.clear(); }
	ProvState State()const { return m_state.load(); }double Prog()const { return m_prog.load(); }std::wstring Path()const { return m_path; }std::string Err()const { std::lock_guard<std::mutex> lk(m_mtx);return m_err; }
	~GenericProvisioner() { if (m_thread.joinable())m_thread.join(); }
private:
	void Work() {
		size_t slash = m_path.find_last_of(L"\\/");
		if (slash != std::wstring::npos) { std::error_code ec;std::filesystem::create_directories(m_path.substr(0, slash), ec); }
		if (ValidateModelFile(m_path, m_minBytes)) { m_state = ProvState::Ready;return; }
		DeleteFileW(m_path.c_str());
		for (const auto& url : m_urls) {
			if (url.empty())continue;
			m_state = ProvState::Downloading;
			m_prog.store(0.0);
			bool zipAsset = EndsWithNoCase(url, L".zip");
			std::wstring tmp = m_path + (zipAsset ? L".zip.part" : L".part");
			std::string err;
			if (!DownloadUrlToFile(url, tmp, &m_prog, &err)) { SetErr(err);DeleteFileW(tmp.c_str());continue; }
			m_state = ProvState::Verifying;
			if (zipAsset) {
				std::wstring rootDir = GetRootExtractDirForModelPath(m_path);
				if (!ExtractZipToDirectory(tmp, rootDir)) { SetErr("Zip extract failed");DeleteFileW(tmp.c_str());continue; }
				DeleteFileW(tmp.c_str());
				if (!ValidateModelFile(m_path, m_minBytes)) { SetErr("Extracted ONNX failed validation");continue; }
				m_state = ProvState::Ready;
				return;
			}
			if (!ValidateModelFile(tmp, m_minBytes)) { SetErr("File failed validation");DeleteFileW(tmp.c_str());continue; }
			DeleteFileW(m_path.c_str());
			if (!MoveFileW(tmp.c_str(), m_path.c_str())) { SetErr("MoveFile failed");DeleteFileW(tmp.c_str());continue; }
			m_state = ProvState::Ready;
			return;
		}
		m_state = ProvState::Failed;
	}
	void SetErr(const std::string& msg) { std::lock_guard<std::mutex> lk(m_mtx);m_err = msg; }
	std::thread m_thread;
	std::atomic<bool> m_started{ false };
	std::atomic<ProvState> m_state{ ProvState::Checking };
	std::atomic<double> m_prog{ 0.0 };
	mutable std::mutex m_mtx;
	std::string m_err;
	std::vector<std::wstring> m_urls;
	std::wstring m_path;
	int64_t m_minBytes = 1000000;
};
struct AiModelDef {
	const char* id;const char* displayName;const char* category;
	const wchar_t* url1;const wchar_t* url2;const wchar_t* localPath;int sizeMB;int outputScale;bool needsClipText;const char* defaultPrompt;
};
static const AiModelDef kAiModels[] = {
{"realesrgan_x4plus","Real-ESRGAN x4 Plus","Upscale",L"https://qaihub-public-assets.s3.us-west-2.amazonaws.com/qai-hub-models/models/real_esrgan_x4plus/releases/v0.59.0/real_esrgan_x4plus-onnx-float.zip",L"",L"models\\real_esrgan_x4plus-onnx-float\\real_esrgan_x4plus.onnx",1,3,false,""},
{"realesr_general_x4v3","Real-ESRGAN General x4v3","Upscale",L"https://qaihub-public-assets.s3.us-west-2.amazonaws.com/qai-hub-models/models/real_esrgan_general_x4v3/releases/v0.59.0/real_esrgan_general_x4v3-onnx-float.zip",L"",L"models\\realesr_general_x4v3-onnx-float\\realesr_general_x4v3.onnx",1,3,false,""},
{"quicksr_small","QuickSRNet Small","Upscale",L"https://qaihub-public-assets.s3.us-west-2.amazonaws.com/qai-hub-models/models/quicksrnetsmall/releases/v0.59.0/quicksrnetsmall-onnx-float.zip",L"",L"models\\quicksrnet_small-onnx-float\\quicksrnet_small.onnx",1,3,false,""},
{"quicksr_medium","QuickSRNet Medium","Upscale",L"https://qaihub-public-assets.s3.us-west-2.amazonaws.com/qai-hub-models/models/quicksrnetmedium/releases/v0.59.0/quicksrnetmedium-onnx-float.zip",L"",L"models\\quicksrnetmedium-onnx-float\\quicksrnetmedium.onnx",1,3,false,""},
{"quicksr_large","QuickSRNet Large","Upscale",L"https://qaihub-public-assets.s3.us-west-2.amazonaws.com/qai-hub-models/models/quicksrnetlarge/releases/v0.59.0/quicksrnetlarge-onnx-float.zip",L"",L"models\\quicksrnetlarge-onnx-float\\quicksrnetlarge.onnx",1,3,false,""}
};
constexpr int kNumAiModels = sizeof(kAiModels) / sizeof(kAiModels[0]);
static int g_selModel = 0;
static int64_t GetAiModelMinBytes(const AiModelDef& def) {
	if (strcmp(def.id, "realesrgan_x4plus") == 0)return 400000;
	if (strcmp(def.id, "realesr_general_x4v3") == 0)return 35000;
	if (strcmp(def.id, "quicksr_small") == 0)return 3000;
	if (strcmp(def.id, "quicksr_medium") == 0)return 5000;
	if (strcmp(def.id, "quicksr_large") == 0)return 12000;
	return 3000;
}
struct SdFileSpec {
	const wchar_t* url = nullptr;
	const wchar_t* localPath = nullptr;
	int64_t minBytes = 0;
	int64_t expectedBytes = 0;
	const char* label = "";
	bool isZip = false;
};
static const SdFileSpec kSdFiles[] = {
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/text_encoder/model.onnx",L"models\\sd15\\text_encoder.onnx",200000000LL,246476214LL,"Text encoder(fp16)"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/unet/model.onnx",L"models\\sd15\\model.onnx",1000000LL,1217704LL,"UNet graph"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/unet/weights.pb",L"models\\sd15\\weights.pb",1500000000LL,1718976000LL,"UNet weights(1.7 GB)"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/vae_decoder/model.onnx",L"models\\sd15\\vae_decoder.onnx",80000000LL,99094195LL,"VAE decoder(fp16)"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/vae_encoder/model.onnx",L"models\\sd15\\vae_encoder.onnx",50000000LL,68430493LL,"VAE encoder(fp16)"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/tokenizer/vocab.json",L"models\\sd15\\vocab.json",900000LL,1059962LL,"Token vocab"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/tokenizer/merges.txt",L"models\\sd15\\merges.txt",400000LL,524619LL,"Token merges"},
{L"https://huggingface.co/akameswa/lcm-tiny-sd-onnx-fp16/resolve/main/unet/model.onnx",L"models\\sd15fast\\model.onnx",250000LL,354685LL,"Fast UNet graph(LCM)"},
{L"https://huggingface.co/akameswa/lcm-tiny-sd-onnx-fp16/resolve/main/unet/model.onnx_data",L"models\\sd15fast\\model.onnx_data",600000000LL,647000000LL,"Fast UNet weights(LCM)"},
{L"https://huggingface.co/SimianLuo/LCM_Dreamshaper_v7/resolve/main/unet/model.onnx",L"models\\dreamshaper\\model.onnx",1500000LL,1950000LL,"DreamShaper V7 UNet graph"},
{L"https://huggingface.co/SimianLuo/LCM_Dreamshaper_v7/resolve/main/unet/model.onnx_data",L"models\\dreamshaper\\model.onnx_data",3000000000LL,3440000000LL,"DreamShaper V7 UNet weights"}
};
constexpr int kSdFileCount = (int)(sizeof(kSdFiles) / sizeof(kSdFiles[0]));
static const SdFileSpec kVisionFiles[] = {
{L"https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo11n.onnx",L"models\\vision\\yolo11n.onnx",9000000LL,10930182LL,"YOLO11n(fast)"},
{L"https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo11s.onnx",L"models\\vision\\yolo11s.onnx",30000000LL,38051729LL,"YOLO11s(balanced)"},
{L"https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo11m.onnx",L"models\\vision\\yolo11m.onnx",70000000LL,80673621LL,"YOLO11m(accurate)"},
{L"https://huggingface.co/onnx-community/depth-anything-v2-small/resolve/main/onnx/model.onnx",L"models\\vision\\depth_anything_v2_vits.onnx",90000000LL,99060839LL,"Depth Anything V2(depth)"},
{L"https://huggingface.co/onnx-community/sam2.1-hiera-tiny-ONNX/resolve/main/onnx/vision_encoder_fp16.onnx",L"models\\vision\\sam2_vision_encoder.onnx",150000LL,224883LL,"SAM2.1 vision encoder"},
{L"https://huggingface.co/onnx-community/sam2.1-hiera-tiny-ONNX/resolve/main/onnx/vision_encoder_fp16.onnx_data",L"models\\vision\\sam2_vision_encoder.onnx_data",60000000LL,67005504LL,"SAM2.1 encoder weights"},
{L"https://huggingface.co/onnx-community/sam2.1-hiera-tiny-ONNX/resolve/main/onnx/prompt_encoder_mask_decoder_fp16.onnx",L"models\\vision\\sam2_mask_decoder.onnx",100000LL,156784LL,"SAM2.1 mask decoder"},
{L"https://huggingface.co/onnx-community/sam2.1-hiera-tiny-ONNX/resolve/main/onnx/prompt_encoder_mask_decoder_fp16.onnx_data",L"models\\vision\\sam2_mask_decoder.onnx_data",9000000LL,10454016LL,"SAM2.1 decoder weights"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/text_encoder/model.onnx",L"models\\vision\\clip_text_encoder.onnx",200000000LL,246476214LL,"CLIP text encoder"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/tokenizer/vocab.json",L"models\\vision\\vocab.json",900000LL,1059962LL,"CLIP vocab"},
{L"https://huggingface.co/nmkd/stable-diffusion-1.5-onnx-fp16/resolve/main/tokenizer/merges.txt",L"models\\vision\\merges.txt",400000LL,524619LL,"CLIP merges"}
};
constexpr int kVisionFileCount = (int)(sizeof(kVisionFiles) / sizeof(kVisionFiles[0]));
static int g_radarModelIdx = 1;
static bool VisionFilesReady() {
	for (int i = 0;i < kVisionFileCount;++i)
		if (!ValidateModelFile(kVisionFiles[i].localPath, kVisionFiles[i].minBytes))return false;
	return true;
}
struct ShaderStyle {
	float outlineStrength = 0.0f;
	float bloom = 0.0f;
	float saturation = 1.0f;
	float fogDensity = 0.0f;
	float toonLevels = 0.0f;
	float rimLighting = 0.0f;
	float paletteMix = 0.0f;
	float edgeSoftness = 0.0f;
	float contrast = 1.0f;
	float vignette = 0.0f;
	float grain = 0.0f;
	float chromatic = 0.0f;
	float neonGlow = 0.0f;
	float skyGlow = 0.0f;
	float warmCool = 0.0f;
	float pixelMix = 0.0f;
	float shadowTint[3] = { 0.05f,0.05f,0.1f };
	float highlightTint[3] = { 1.0f,1.0f,1.0f };
	int themeMode = 0;
	int tonemapMode = 0;
	bool outlineEnabled = false;
	bool fogEnabled = false;
	bool pixelEnabled = false;
	float rain = 0.0f;
	float wind = 0.0f;
	std::string name;
};
struct StylePresetDef {
	const char* name;
	const char* clipPrompt;
	ShaderStyle s;
};
static ShaderStyle MakeStyle(float outline, float bloom, float sat, float fog, float toon, float rim,
	float palette, float edge, float contrast, float vig, float grain,
	float chroma, float neon, float sky, float warm, float pixel,
	const float* shT, const float* hiT, int theme, int tonemap) {
	ShaderStyle st;
	st.outlineStrength = outline;st.bloom = bloom;st.saturation = sat;st.fogDensity = fog;
	st.toonLevels = toon;st.rimLighting = rim;st.paletteMix = palette;st.edgeSoftness = edge;
	st.contrast = contrast;st.vignette = vig;st.grain = grain;st.chromatic = chroma;
	st.neonGlow = neon;st.skyGlow = sky;st.warmCool = warm;st.pixelMix = pixel;
	if (shT) { st.shadowTint[0] = shT[0];st.shadowTint[1] = shT[1];st.shadowTint[2] = shT[2]; }
	if (hiT) { st.highlightTint[0] = hiT[0];st.highlightTint[1] = hiT[1];st.highlightTint[2] = hiT[2]; }
	st.themeMode = theme;st.tonemapMode = tonemap;
	st.outlineEnabled = outline > 0.1f;st.fogEnabled = fog > 0.02f;st.pixelEnabled = pixel > 0.1f;
	return st;
}
static const float kBlue[] = { 0.05f,0.1f,0.4f }, kPink[] = { 1.0f,0.3f,0.85f };
static const float kBlack[] = { 0.02f,0.02f,0.05f }, kPaper[] = { 0.95f,0.93f,0.88f };
static const float kWarm[] = { 0.3f,0.2f,0.1f }, kGold[] = { 1.0f,0.85f,0.55f };
static const float kCool[] = { 0.1f,0.15f,0.35f }, kIce[] = { 0.75f,0.9f,1.0f };
static const float kGreen[] = { 0.05f,0.2f,0.1f }, kEmber[] = { 1.0f,0.5f,0.2f };
struct ConceptDef {
	const char* name;
	const char* clipText;
	ShaderStyle d;
	float prior;
};
static ShaderStyle MakeDelta(float outline, float bloom, float sat, float fog, float toon, float rim,
	float palette, float edge, float contrast, float vig, float grain,
	float chroma, float neon, float sky, float warm, float pixel,
	float rain, float wind,
	const float* shT, const float* hiT, int theme, int tonemap) {
	ShaderStyle st;
	st.outlineStrength = outline;st.bloom = bloom;st.saturation = sat;st.fogDensity = fog;
	st.toonLevels = toon;st.rimLighting = rim;st.paletteMix = palette;st.edgeSoftness = edge;
	st.contrast = contrast;st.vignette = vig;st.grain = grain;st.chromatic = chroma;
	st.neonGlow = neon;st.skyGlow = sky;st.warmCool = warm;st.pixelMix = pixel;
	st.rain = rain;st.wind = wind;
	if (shT) { st.shadowTint[0] = shT[0];st.shadowTint[1] = shT[1];st.shadowTint[2] = shT[2]; }
	if (hiT) { st.highlightTint[0] = hiT[0];st.highlightTint[1] = hiT[1];st.highlightTint[2] = hiT[2]; }
	st.themeMode = theme;st.tonemapMode = tonemap;
	st.outlineEnabled = outline > 0.1f;st.fogEnabled = fog > 0.02f;st.pixelEnabled = pixel > 0.1f;
	return st;
}
static ShaderStyle NeutralStyle() {
	ShaderStyle st;
	st.saturation = 1.0f;st.contrast = 1.0f;
	st.shadowTint[0] = 0.05f;st.shadowTint[1] = 0.05f;st.shadowTint[2] = 0.1f;
	st.highlightTint[0] = 1;st.highlightTint[1] = 1;st.highlightTint[2] = 1;
	return st;
}
static const float kNeonSh[] = { 0.03f,0.08f,0.35f }, kNeonHi[] = { 1.0f,0.35f,0.9f };
static const float kSunSh[] = { 0.25f,0.15f,0.08f }, kSunHi[] = { 1.0f,0.85f,0.6f };
static const float kNightSh[] = { 0.02f,0.03f,0.12f }, kNightHi[] = { 0.6f,0.7f,1.0f };
static const float kNoirSh[] = { 0.02f,0.02f,0.04f }, kNoirHi[] = { 0.95f,0.93f,0.9f };
static const float kGhibliSh[] = { 0.2f,0.12f,0.05f }, kGhibliHi[] = { 1.0f,0.9f,0.65f };
static const float kPastelSh[] = { 0.5f,0.45f,0.55f }, kPastelHi[] = { 1.0f,0.95f,1.0f };
static const float kCoolSh[] = { 0.06f,0.1f,0.3f }, kCoolHi[] = { 0.8f,0.9f,1.0f };
static const float kWarmSh[] = { 0.3f,0.18f,0.08f }, kWarmHi[] = { 1.0f,0.9f,0.7f };
static const float kHorrorSh[] = { 0.02f,0.01f,0.02f }, kHorrorHi[] = { 0.7f,0.75f,0.8f };
static const float kForestSh[] = { 0.05f,0.15f,0.08f }, kForestHi[] = { 0.85f,1.0f,0.8f };
static const float kOceanSh[] = { 0.03f,0.1f,0.3f }, kOceanHi[] = { 0.7f,0.95f,1.0f };
static const float kIceHi[] = { 0.8f,0.95f,1.0f };
static const ConceptDef kConcepts[] = {
{"anime","anime style,cel shaded,vibrant anime artwork,clean lines",MakeDelta(0.85f,0.2f,0.25f,0,3,0.45f,0,0.05f,0.05f,0.05f,0,0,0.1f,0.08f,0.15f,0,0,0,kCoolSh,kWarmHi,9,1),2.0f},
{"cel shaded","cel shading,flat colors,cartoon shading,crisp bands",MakeDelta(1.0f,0.1f,0.22f,0,4,0.4f,0,0,0.1f,0,0,0,0,0,0.1f,0,0,0,kCoolSh,kWarmHi,9,1),1.0f},
{"comic","comic book art,halftone dots,bold ink outlines,pop art",MakeDelta(1.2f,0.08f,0.18f,0,5,0.3f,0.55f,0,0.25f,0.05f,0.2f,0.08f,0,0,0,0,0,0,kNoirSh,kNoirHi,8,2),1.0f},
{"manga","manga,japanese comic,screentone,ink lines,black and white",MakeDelta(1.3f,0,0.1f,0,5,0.25f,0.7f,0,0.3f,0.05f,0.15f,0,0,0,-0.1f,0,0,0,kNoirSh,kNoirHi,8,2),1.2f},
{"watercolor","watercolor painting,soft washes,paper texture,gentle colors",MakeDelta(0.1f,0,0.08f,0.05f,0,0,0.1f,0.6f,-0.1f,0.22f,0.06f,0,0,0,0.05f,0,0,0,kCoolSh,kPastelHi,4,0),1.2f},
{"painterly","oil painting,impasto brush strokes,rich texture,artistic",MakeDelta(0.1f,0.08f,0.1f,0.05f,0,0.15f,0.05f,0.7f,0.1f,0.1f,0.08f,0,0,0,0,0,0,0,kWarmSh,kSunHi,4,1),1.0f},
{"pixel art","pixel art,retro 8-bit game,blocky pixels,low resolution",MakeDelta(0.6f,0,0.3f,0,8,0.2f,0.8f,0,0.1f,0,0,0,0,0,0,0.9f,0,0,kNoirSh,kWarmHi,17,0),1.3f},
{"noir","film noir,black and white,hard shadows,dramatic",MakeDelta(0.7f,0,0.2f,0.18f,0,0.3f,0.6f,0,0.35f,0.4f,0.3f,0,0,0,-0.1f,0,0,0,kNoirSh,kNoirHi,8,2),1.3f},
{"cyberpunk","cyberpunk,neon lit city,pink and blue neon lights,rainy night",MakeDelta(0.45f,0.85f,0.25f,0.12f,0,0.35f,0.25f,0,0.05f,0.15f,0.18f,0.12f,0.95f,0.25f,-0.2f,0,0.15f,0.1f,kNeonSh,kNeonHi,3,1),1.8f},
{"neon","neon lights,glowing signs,vibrant glow,night city",MakeDelta(0.3f,0.9f,0.3f,0.05f,0,0.3f,0.15f,0,0.05f,0.1f,0.1f,0.08f,1.1f,0.2f,-0.15f,0,0,0,kNeonSh,kNeonHi,3,1),1.4f},
{"fantasy","fantasy,magical glowing light,enchanted,ethereal",MakeDelta(0.35f,0.4f,0.2f,0.08f,0,0.55f,0.08f,0.12f,0.05f,0.1f,0.04f,0,0.2f,0.4f,0.2f,0,0,0,kWarmSh,kSunHi,9,1),1.0f},
{"cinematic","cinematic film look,movie lighting,dramatic,high contrast",MakeDelta(0.15f,0.15f,0.05f,0.08f,0,0.25f,0,0,0.15f,0.3f,0.12f,0.05f,0,0.1f,0.05f,0,0,0,kCoolSh,kWarmHi,10,1),1.0f},
{"retro","retro game,vintage,ps2 era graphics,nostalgic",MakeDelta(0.4f,0.05f,0.1f,0.12f,2,0.1f,0.25f,0.4f,0.05f,0.1f,0.15f,0,0,0,0,0,0,0,kCoolSh,kWarmHi,17,0),1.0f},
{"ghibli","studio ghibli,warm whimsical colors,soft light,gentle",MakeDelta(0.2f,0.3f,0.25f,0.04f,0,0.4f,0,0.25f,0,0,0,0,0,0.45f,0.3f,0,0,0,kGhibliSh,kGhibliHi,12,1),1.5f},
{"minecraft","minecraft,voxel world,blocky cubes,default texture pack",MakeDelta(0.5f,0,0.25f,0,8,0.15f,0.7f,0,0.15f,0,0,0,0,0,0,0.85f,0,0,kForestSh,kWarmHi,17,0),1.5f},
{"rain","rainy weather,rain streaks,wet ground,overcast,puddles",MakeDelta(0.1f,0.15f,0.05f,0.1f,0,0.1f,0,0,0.05f,0.05f,0.05f,0,0,0,-0.12f,0,0.7f,0.35f,kCoolSh,kCoolHi,13,0),2.0f},
{"storm","storm,heavy rain,dark clouds,dramatic sky,lightning",MakeDelta(0.2f,0.2f,0.05f,0.2f,0,0.15f,0,0,0.2f,0.1f,0.08f,0,0.1f,0,-0.2f,0,0.95f,0.7f,kNoirSh,kCoolHi,13,0),1.8f},
{"snow","snowfall,winter,cold,white snow,icy",MakeDelta(0.1f,0.05f,0.08f,0.15f,0,0.1f,0,0.1f,0.1f,0.05f,0.05f,0,0,0.05f,-0.25f,0,0.5f,0.2f,kCoolSh,kIceHi,16,0),1.2f},
{"fog","foggy,misty,atmospheric haze,low visibility",MakeDelta(0.05f,0.05f,0,0.45f,0,0.05f,0,0.2f,0.05f,0.15f,0.05f,0,0,0.05f,-0.05f,0,0,0,kCoolSh,kCoolHi,7,0),1.2f},
{"night","night time,dark scene,moonlight,stars",MakeDelta(0.2f,0.15f,0.1f,0.08f,0,0.2f,0,0,0.12f,0.15f,0.08f,0.03f,0.2f,0.1f,-0.25f,0,0,0,kNightSh,kNightHi,3,1),1.6f},
{"sunset","sunset,golden hour,warm orange light,beautiful sky",MakeDelta(0.1f,0.25f,0.15f,0.04f,0,0.25f,0,0,0.05f,0.1f,0.05f,0,0,0.3f,0.35f,0,0,0,kSunSh,kSunHi,6,1),1.5f},
{"sunny","sunny day,bright daylight,clear sky,cheerful",MakeDelta(0,0.05f,0.15f,0,0,0.1f,0,0,0,0,0,0,0,0.15f,0.15f,0,0,0,kWarmSh,kSunHi,4,1),0.8f},
{"dark","dark,gloomy,moody,shadows,low key",MakeDelta(0.2f,0,0.05f,0.1f,0,0.15f,0,0,0.25f,0.3f,0.15f,0,0.05f,0,-0.15f,0,0,0,kNoirSh,kCoolHi,8,2),1.2f},
{"vibrant","vibrant,colorful,saturated,vivid,punchy colors",MakeDelta(0,0.1f,0.35f,0,0,0.1f,0,0,0.05f,0,0.05f,0,0.05f,0.05f,0.05f,0,0,0,kCoolSh,kWarmHi,21,1),1.2f},
{"pastel","pastel colors,soft,gentle,light,dreamy",MakeDelta(0,0.1f,0.05f,0.04f,0,0.1f,0,0.2f,-0.05f,0.1f,0,0,0,0.15f,0.1f,0,0,0,kPastelSh,kPastelHi,12,0),1.0f},
{"warm","warm tones,cozy,golden light,warm color grade",MakeDelta(0,0.1f,0.05f,0.02f,0,0.1f,0,0,0,0.05f,0,0,0,0.1f,0.3f,0,0,0,kWarmSh,kSunHi,6,1),1.0f},
{"cool","cool tones,cold,blue,teal,icy color grade",MakeDelta(0,0.05f,0.05f,0.02f,0,0.1f,0,0,0.05f,0.05f,0,0,0.05f,0.05f,-0.3f,0,0,0,kCoolSh,kIceHi,16,0),1.0f},
{"horror","horror,scary,dark,creepy,desaturated,unsettling",MakeDelta(0.3f,0,0.1f,0.15f,0,0.2f,0.2f,0,0.3f,0.4f,0.25f,0.08f,0,0,-0.2f,0,0,0,kHorrorSh,kHorrorHi,8,2),1.2f},
{"city","city streets,urban,buildings,street lights",MakeDelta(0.1f,0.2f,0.1f,0.05f,0,0.15f,0.05f,0,0.05f,0.1f,0.08f,0.05f,0.2f,0.1f,-0.1f,0,0.1f,0.15f,kNeonSh,kNeonHi,3,1),0.8f},
{"forest","forest,trees,nature,green,woodland",MakeDelta(0.1f,0.15f,0.15f,0.06f,0,0.2f,0,0.05f,0.05f,0.1f,0.05f,0,0,0.2f,0.05f,0,0,0,kForestSh,kForestHi,18,1),0.8f},
{"ocean","ocean,sea,beach,underwater,blue water",MakeDelta(0.05f,0.2f,0.15f,0.04f,0,0.15f,0,0.05f,0.05f,0.1f,0.05f,0,0,0.25f,-0.2f,0,0,0,kOceanSh,kOceanHi,16,1),0.8f},
{"desert","desert,sand,dunes,dry,hot",MakeDelta(0.1f,0.15f,0.1f,0.04f,0,0.2f,0,0.1f,0.1f,0.1f,0.05f,0,0,0.2f,0.3f,0,0,0,kWarmSh,kSunHi,6,1),0.8f},
{"dreamy","dreamy,ethereal,soft focus,magical,whimsical",MakeDelta(0,0.3f,0.15f,0.06f,0,0.35f,0,0.35f,0,0.15f,0.05f,0,0.1f,0.3f,0.1f,0,0,0,kPastelSh,kPastelHi,12,1),0.9f},
{"sketch","pencil sketch,line art,hand drawn,ink drawing",MakeDelta(1.4f,0,0.1f,0,6,0.2f,0.75f,0,0.2f,0.05f,0.1f,0,0,0,0,0,0,0,kNoirSh,kNoirHi,8,2),1.2f},
{"realistic","realistic,natural,photo,real life,no stylization",MakeDelta(0.05f,0.05f,0.02f,0.03f,0,0.1f,0,0,0.1f,0.15f,0.08f,0.03f,0,0.05f,0,0,0,0,kCoolSh,kWarmHi,10,1),1.0f},
};
static constexpr int kConceptCount = (int)(sizeof(kConcepts) / sizeof(kConcepts[0]));
static const StylePresetDef kStylePresets[] = {
{"Anime","anime style,cel shading,clean outlines,vibrant colors",
 MakeStyle(0.9f,0.25f,1.35f,0.0f,3,0.5f,0.0f,0.0f,1.05f,0.05f,0.0f,0.0f,0.1f,0.1f,0.2f,0.0f,kCool,kWarm,9,1)},
{"Cel Shaded","cel shaded,flat colors,crisp edges,cartoon",
 MakeStyle(1.1f,0.15f,1.3f,0.0f,4,0.45f,0.0f,0.0f,1.1f,0.0f,0.0f,0.0f,0.0f,0.0f,0.1f,0.0f,kCool,kWarm,9,1)},
{"Comic","comic book,halftone,bold ink,high contrast",
 MakeStyle(1.3f,0.1f,1.2f,0.0f,5,0.3f,0.55f,0.0f,1.25f,0.05f,0.25f,0.1f,0.0f,0.0f,0.0f,0.0f,kBlack,kPaper,8,2)},
{"Cyberpunk","cyberpunk,neon lights,blue shadows,pink highlights,rain",
 MakeStyle(0.5f,0.9f,1.25f,0.15f,0,0.4f,0.25f,0.0f,1.05f,0.15f,0.2f,0.15f,0.9f,0.3f,-0.2f,0.0f,kBlue,kPink,3,1)},
{"Watercolor","watercolor painting,soft washes,paper texture",
 MakeStyle(0.15f,0.0f,0.9f,0.05f,0,0.0f,0.12f,0.6f,0.85f,0.25f,0.08f,0.0f,0.0f,0.0f,0.05f,0.0f,kCool,kPaper,4,0)},
{"Painterly","oil painting,impasto,rich brush strokes",
 MakeStyle(0.1f,0.1f,1.1f,0.05f,0,0.15f,0.05f,0.7f,1.1f,0.1f,0.1f,0.0f,0.0f,0.0f,0.0f,0.0f,kWarm,kGold,4,1)},
{"Pixel Art","pixel art,retro game,blocky,8-bit",
 MakeStyle(0.6f,0.0f,1.3f,0.0f,8,0.2f,0.8f,0.0f,1.1f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.9f,kBlack,kWarm,17,0)},
{"Noir","noir film,black and white,hard shadows,film grain",
 MakeStyle(0.7f,0.0f,0.55f,0.2f,0,0.3f,0.6f,0.0f,1.35f,0.45f,0.35f,0.0f,0.0f,0.0f,-0.1f,0.0f,kBlack,kPaper,8,2)},
{"Fantasy","fantasy,magical glow,golden light,ethereal",
 MakeStyle(0.4f,0.45f,1.2f,0.1f,0,0.6f,0.1f,0.15f,1.05f,0.1f,0.05f,0.0f,0.2f,0.45f,0.25f,0.0f,kWarm,kGold,9,1)},
{"Cinematic","cinematic film,realistic,dramatic lighting,movie",
 MakeStyle(0.2f,0.15f,1.05f,0.08f,0,0.25f,0.0f,0.0f,1.15f,0.3f,0.15f,0.05f,0.0f,0.1f,0.05f,0.0f,kCool,kWarm,10,1)},
{"Retro PS2","ps2 era game,low fidelity,retro render",
 MakeStyle(0.45f,0.05f,0.95f,0.12f,2,0.1f,0.25f,0.45f,1.05f,0.1f,0.2f,0.0f,0.0f,0.0f,0.0f,0.0f,kCool,kWarm,17,0)},
{"Ghibli","studio ghibli,warm colors,soft light,whimsical",
 MakeStyle(0.25f,0.35f,1.25f,0.05f,0,0.45f,0.0f,0.25f,1.0f,0.0f,0.0f,0.0f,0.0f,0.5f,0.3f,0.0f,kWarm,kGold,12,1)},
{"Minecraft","minecraft,voxel,blocky,cubic,default texture pack",
 MakeStyle(0.5f,0.0f,1.25f,0.0f,8,0.15f,0.7f,0.0f,1.15f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.85f,kGreen,kEmber,17,0)},
};
static constexpr int kStylePresetCount = (int)(sizeof(kStylePresets) / sizeof(kStylePresets[0]));
static float KeywordBoost(const std::string& lower, const char* kw) {
	return lower.find(kw) != std::string::npos ? 1.0f : 0.0f;
}
static void ApplySceneToStyle(ShaderStyle& st, bool hasPerson, bool hasVehicle, float skyFrac, float groundFrac) {
	if (hasPerson) { st.outlineStrength = std::min(2.0f, st.outlineStrength + 0.2f);st.rimLighting = std::min(1.0f, st.rimLighting + 0.25f); }
	if (hasVehicle) { st.neonGlow = std::min(2.0f, st.neonGlow + 0.2f);st.bloom = std::min(2.0f, st.bloom + 0.08f); }
	if (skyFrac > 0.25f) { st.skyGlow = std::min(2.0f, st.skyGlow + 0.2f); }
	if (groundFrac > 0.35f && st.fogDensity > 0.01f) { st.fogDensity = std::min(1.0f, st.fogDensity + 0.06f); }
}
static const SdFileSpec kDsFiles[] = {
{L"https://huggingface.co/SimianLuo/LCM_Dreamshaper_v7/resolve/main/unet/model.onnx",L"models\\dreamshaper\\model.onnx",1500000LL,1950000LL,"DreamShaper V7 UNet graph"},
{L"https://huggingface.co/SimianLuo/LCM_Dreamshaper_v7/resolve/main/unet/model.onnx_data",L"models\\dreamshaper\\model.onnx_data",3000000000LL,3440000000LL,"DreamShaper V7 UNet weights"}
};
static const SdFileSpec kMbFiles[] = {
{L"https://huggingface.co/simonw/Moebius-ONNX/resolve/main/unet.onnx",L"models\\moebius\\unet.onnx",800000000LL,906698976LL,"Moebius UNet(0.22B)"},
{L"https://huggingface.co/simonw/Moebius-ONNX/resolve/main/vae_encoder.onnx",L"models\\moebius\\vae_encoder.onnx",120000000LL,136757093LL,"Moebius VAE encoder"},
{L"https://huggingface.co/simonw/Moebius-ONNX/resolve/main/vae_decoder.onnx",L"models\\moebius\\vae_decoder.onnx",170000000LL,198078671LL,"Moebius VAE decoder"}
};
constexpr int kDsFileCount = (int)(sizeof(kDsFiles) / sizeof(kDsFiles[0]));
enum class SdProvState { Idle, Downloading, Ready, Failed };
class SdProvisioner {
public:
	void Start(const SdFileSpec* specs, int count) {
		bool expected = false;
		if (!m_running.compare_exchange_strong(expected, true))return;
		m_specs = specs;
		m_count = count;
		m_state = SdProvState::Downloading;
		m_err.clear();
		for (int i = 0;i < count;++i) {
			m_fileTotal[i].store(0);
			m_fileBytes[i].store(0);
			m_fileDone[i].store(false);
		}
		m_thread = std::thread([this] {SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);Work();});
	}
	void Cancel() { m_cancel.store(true); }
	void Reset() {
		m_cancel.store(false);
		if (m_thread.joinable())m_thread.join();
		m_running.store(false);
		m_state = SdProvState::Idle;
		m_err.clear();
	}
	SdProvState State()const { return m_state.load(); }
	bool Running()const { return m_running.load(); }
	double OverallProg()const {
		int64_t d = 0, t = 0;
		for (int i = 0;i < m_count;++i) { d += m_fileBytes[i].load();t += m_fileTotal[i].load(); }
		return t > 0 ? std::min(1.0, (double)d / (double)t) : 0.0;
	}
	double DoneBytes()const { int64_t d = 0;for (int i = 0;i < m_count;++i)d += m_fileBytes[i].load();return (double)d; }
	double TotalBytes()const { int64_t t = 0;for (int i = 0;i < m_count;++i)t += m_fileTotal[i].load();return (double)t; }
	double FileProg(int i)const { int64_t t = m_fileTotal[i].load();return t > 0 ? std::min(1.0, (double)m_fileBytes[i].load() / (double)t) : 0.0; }
	bool FileDone(int i)const { return m_fileDone[i].load(); }
	std::string Err()const { return m_err; }
	~SdProvisioner() { m_cancel.store(true);if (m_thread.joinable())m_thread.join(); }
private:
	void Work() {
		std::error_code ec;
		std::filesystem::create_directories(L"models\\sd15", ec);
		std::filesystem::create_directories(L"models\\sd15fast", ec);
		std::filesystem::create_directories(L"models\\dreamshaper", ec);
		std::filesystem::create_directories(L"models\\vision", ec);
		std::filesystem::create_directories(L"models\\moebius", ec);
		std::filesystem::create_directories(L"models\\brain", ec);
		for (int i = 0;i < m_count;++i) {
			std::wstring lp(m_specs[i].localPath);
			size_t sl = lp.find_last_of(L"\\/");
			if (sl != std::wstring::npos)std::filesystem::create_directories(lp.substr(0, sl), ec);
		}
		ULARGE_INTEGER freeAvail{};
		if (GetDiskFreeSpaceExW(nullptr, &freeAvail, nullptr, nullptr)) {
			if (freeAvail.QuadPart < 3000000000LL) {
				m_err = "Not enough free disk space.";
				m_state = SdProvState::Failed;
				m_running.store(false);
				return;
			}
		}
		for (int i = 0;i < m_count;++i) {
			int64_t s = QueryRemoteSize(m_specs[i].url);
			if (s <= 0)s = m_specs[i].expectedBytes;
			m_fileTotal[i].store(s);
		}
		for (int i = 0;i < m_count;++i) {
			if (ValidateModelFile(m_specs[i].localPath, m_specs[i].minBytes)) {
				m_fileBytes[i].store(m_fileTotal[i].load());
				m_fileDone[i].store(true);
			}
		}
		bool allDone = true;
		for (int i = 0;i < m_count;++i)if (!m_fileDone[i].load())allDone = false;
		if (allDone) { m_state = SdProvState::Ready;m_running.store(false);return; }
		std::vector<std::thread> threads;
		for (int i = 0;i < m_count;++i) {
			if (m_fileDone[i].load())continue;
			threads.emplace_back([this, i] {
				std::string err;
				if (m_specs[i].isZip) {
					std::wstring tmpZip = std::wstring(m_specs[i].localPath) + L".zip.part";
					if (!DownloadFileSmart(m_specs[i].url, tmpZip, m_fileTotal[i].load(), &m_fileBytes[i], &m_cancel, &err)) {
						m_failMtx.lock();
						if (m_err.empty())m_err = std::string(m_specs[i].label) + ": " + err;
						m_failMtx.unlock();
						DeleteFileW(tmpZip.c_str());
						return;
					}
					std::wstring rootDir = L"models";
					if (!ExtractZipToDirectory(tmpZip, rootDir)) {
						m_failMtx.lock();
						if (m_err.empty())m_err = std::string(m_specs[i].label) + ": extraction failed";
						m_failMtx.unlock();
						DeleteFileW(tmpZip.c_str());
						return;
					}
					DeleteFileW(tmpZip.c_str());
					if (!ValidateModelFile(m_specs[i].localPath, m_specs[i].minBytes)) {
						m_failMtx.lock();
						if (m_err.empty())m_err = std::string(m_specs[i].label) + ": extracted file failed validation";
						m_failMtx.unlock();
						return;
					}
					m_fileBytes[i].store(m_fileTotal[i].load());
					m_fileDone[i].store(true);
					return;
				}
				if (!DownloadFileSmart(m_specs[i].url, m_specs[i].localPath, m_fileTotal[i].load(), &m_fileBytes[i], &m_cancel, &err)) {
					m_failMtx.lock();
					if (m_err.empty())m_err = std::string(m_specs[i].label) + ": " + err;
					m_failMtx.unlock();
					return;
				}
				if (!ValidateModelFile(m_specs[i].localPath, m_specs[i].minBytes)) {
					m_failMtx.lock();
					if (m_err.empty())m_err = std::string(m_specs[i].label) + ": downloaded file failed validation";
					m_failMtx.unlock();
					return;
				}
				m_fileBytes[i].store(m_fileTotal[i].load());
				m_fileDone[i].store(true);
				});
		}
		for (auto& t : threads)t.join();
		if (m_cancel.load()) { m_state = SdProvState::Idle;m_running.store(false);return; }
		if (!m_err.empty()) { m_state = SdProvState::Failed;m_running.store(false);return; }
		m_state = SdProvState::Ready;
		m_running.store(false);
	}
	std::thread m_thread;
	std::atomic<bool> m_running{ false };
	std::atomic<bool> m_cancel{ false };
	std::atomic<SdProvState> m_state{ SdProvState::Idle };
	std::atomic<int64_t> m_fileTotal[kSdFileCount]{};
	std::atomic<int64_t> m_fileBytes[kSdFileCount]{};
	std::atomic<bool> m_fileDone[kSdFileCount]{};
	std::mutex m_failMtx;
	std::string m_err;
	const SdFileSpec* m_specs = nullptr;
	int m_count = 0;
};
static bool SdDsReady() {
	for (int i = 0;i < kDsFileCount;++i)
		if (!ValidateModelFile(kDsFiles[i].localPath, kDsFiles[i].minBytes))return false;
	return true;
}
constexpr int kMbFileCount = (int)(sizeof(kMbFiles) / sizeof(kMbFiles[0]));
static const SdFileSpec kBrainFiles[] = {
{L"https://huggingface.co/onnx-community/Qwen2.5-0.5B-Instruct/resolve/main/onnx/model_fp16.onnx",L"models\\brain\\qwen2.5-0.5b-fp16.onnx",900000000LL,997354499LL,"Qwen 0.5B(fp16 GPU)"},
{L"https://huggingface.co/onnx-community/Qwen2.5-0.5B-Instruct/resolve/main/tokenizer.json",L"models\\brain\\tokenizer.json",5000000LL,7031673LL,"Qwen tokenizer"}
};
constexpr int kBrainFileCount = (int)(sizeof(kBrainFiles) / sizeof(kBrainFiles[0]));
static bool BrainFilesReady() {
	for (int i = 0;i < kBrainFileCount;++i)
		if (!ValidateModelFile(kBrainFiles[i].localPath, kBrainFiles[i].minBytes))return false;
	return true;
}
static bool MoebiusReady() {
	for (int i = 0;i < kMbFileCount;++i)
		if (!ValidateModelFile(kMbFiles[i].localPath, kMbFiles[i].minBytes))return false;
	return true;
}
static int g_liveModel = 0;
static SdProvisioner g_dsProv;
#if SR_HAS_ONNX
static std::string OrtSessionInputName(Ort::Session& s, size_t i) {
	Ort::AllocatorWithDefaultOptions a;
	auto p = s.GetInputNameAllocated(i, a);
	return p ? std::string(p.get()) : std::string();
}
static std::string OrtSessionOutputName(Ort::Session& s, size_t i) {
	Ort::AllocatorWithDefaultOptions a;
	auto p = s.GetOutputNameAllocated(i, a);
	return p ? std::string(p.get()) : std::string();
}
static float Fp16ToF32(uint16_t h) {
	uint32_t sign = ((uint32_t)h & 0x8000u) << 16;
	uint32_t exp = ((uint32_t)h >> 10) & 0x1Fu;
	uint32_t man = (uint32_t)h & 0x3FFu;
	uint32_t f;
	if (exp == 0) {
		if (man == 0)f = sign;
		else {
			int32_t e = 127 - 15 + 1;
			while (!(man & 0x400u)) { man <<= 1;--e; }
			man &= 0x3FFu;
			f = sign | ((uint32_t)e << 23) | (man << 13);
		}
	}
	else if (exp == 31) {
		f = sign | 0x7F800000u | (man << 13);
	}
	else {
		f = sign | ((exp + 127 - 15) << 23) | (man << 13);
	}
	float out;
	memcpy(&out, &f, 4);
	return out;
}
static uint16_t F32ToF16(float f) {
	uint32_t x;
	memcpy(&x, &f, 4);
	uint32_t sign = (x >> 16) & 0x8000u;
	int32_t exp = (int32_t)((x >> 23) & 0xFFu) - 127 + 15;
	uint32_t man = x & 0x7FFFFFu;
	uint16_t h;
	if (exp >= 31)h = (uint16_t)(sign | 0x7C00u);
	else if (exp <= 0) {
		if (exp < -10)h = (uint16_t)sign;
		else {
			man |= 0x800000u;
			uint32_t shift = (uint32_t)(14 - exp);
			uint32_t half = man >> shift;
			uint32_t rem = man & ((1u << shift) - 1);
			if (rem + (half & 1u) > (1u << (shift - 1)))++half;
			if (half == 0x400u)h = (uint16_t)(sign | 0x0400u);
			else h = (uint16_t)(sign | half);
		}
	}
	else {
		uint32_t half = man >> 13;
		uint32_t rem = man & 0x1FFFu;
		if (rem + (half & 1u) > 0x1000u)++half;
		if (half == 0x400u) { half = 0;++exp; }
		if (exp >= 31)h = (uint16_t)(sign | 0x7C00u);
		else h = (uint16_t)(sign | ((uint32_t)exp << 10) | half);
	}
	return h;
}
static bool ValueIsFp16(Ort::Value& v) {
	return v.GetTensorTypeAndShapeInfo().GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
}
static bool SessionInputIsFp16(Ort::Session& s, size_t idx) {
	auto ti = s.GetInputTypeInfo(idx);
	return ti.GetTensorTypeAndShapeInfo().GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
}
static std::vector<float> ToF32FromFp16(const uint16_t* h, size_t n) {
	std::vector<float> out(n);
	for (size_t i = 0;i < n;++i)out[i] = Fp16ToF32(h[i]);
	return out;
}
static std::vector<uint16_t> ToFp16FromF32(const float* f, size_t n) {
	std::vector<uint16_t> out(n);
	for (size_t i = 0;i < n;++i)out[i] = F32ToF16(f[i]);
	return out;
}
static Ort::Value MakeFp16Tensor(const Ort::MemoryInfo& mi, uint16_t* data, size_t count, const int64_t* shape, size_t ndim) {
	OrtValue* raw = nullptr;
	OrtStatus* st = Ort::GetApi().CreateTensorWithDataAsOrtValue(static_cast<const OrtMemoryInfo*>(mi), data, count * sizeof(uint16_t), shape, ndim, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16, &raw);
	if (st) { Ort::GetApi().ReleaseStatus(st);return Ort::Value(nullptr); }
	return Ort::Value(raw);
}
enum Ep { EpUnknown, EpCuda, EpTensorrt, EpDml, EpCpu };
static std::atomic<bool> g_epBroken{ false };
static std::mutex g_onnxMtx;
typedef OrtStatus* (ORT_API_CALL* FnAppendEp)(OrtSessionOptions*, int);
static bool AppendEpByName(Ort::SessionOptions& opts, Ep ep) {
	HMODULE h = GetModuleHandleW(L"onnxruntime.dll");
	if (!h)h = LoadLibraryW(L"onnxruntime.dll");
	if (!h)return false;
	const char* fn = nullptr;
	switch (ep) {
	case EpTensorrt: fn = "OrtSessionOptionsAppendExecutionProvider_Tensorrt";break;
	case EpCuda: fn = "OrtSessionOptionsAppendExecutionProvider_CUDA";break;
	case EpDml: fn = "OrtSessionOptionsAppendExecutionProvider_DML";break;
	default: return false;
	}
	FnAppendEp f = (FnAppendEp)GetProcAddress(h, fn);
	if (!f)return false;
	OrtStatus* st = f(static_cast<OrtSessionOptions*>(opts), 0);
	if (st) { Ort::GetApi().ReleaseStatus(st);return false; }
	return true;
}
static const char* OrtVersionStr() {
	static char buf[64] = { 0 };
	if (!buf[0]) {
		HMODULE h = GetModuleHandleW(L"onnxruntime.dll");
		if (!h)h = LoadLibraryW(L"onnxruntime.dll");
		if (h) {
			typedef const char* (ORT_API_CALL* FnV)(void);
			FnV f = (FnV)GetProcAddress(h, "OrtGetVersionString");
			if (f) { const char* s = f();if (s && s[0]) { size_t sl = strlen(s);if (sl >= sizeof(buf))sl = sizeof(buf) - 1;memcpy(buf, s, sl);buf[sl] = 0; } }
		}
		if (!buf[0])memcpy(buf, "unknown", 8);
	}
	return buf;
}
static void OrtMajorMinor(int& major, int& minor) {
	major = 0;minor = 0;
	const char* p = OrtVersionStr();
	if (p && p[0] && p[0] != 'u') {
		while (*p && (*p < '0' || *p > '9'))++p;
		while (*p >= '0' && *p <= '9') { major = major * 10 + (*p - '0');++p; }
		if (*p == '.') { ++p;while (*p >= '0' && *p <= '9') { minor = minor * 10 + (*p - '0');++p; } }
	}
#if defined(_MSC_VER)
	if (major <= 0) {
		HMODULE h = GetModuleHandleW(L"onnxruntime.dll");
		wchar_t dllPath[MAX_PATH] = {};
		if (h && GetModuleFileNameW(h, dllPath, MAX_PATH)) {
			DWORD h2 = 0;DWORD vsz = GetFileVersionInfoSizeW(dllPath, &h2);
			if (vsz) {
				std::vector<uint8_t> vbuf(vsz);
				if (GetFileVersionInfoW(dllPath, h2, vsz, vbuf.data())) {
					VS_FIXEDFILEINFO* ffi = nullptr;UINT fl = 0;
					if (VerQueryValueW(vbuf.data(), L"\\", (void**)&ffi, &fl) && ffi) {
						major = HIWORD(ffi->dwFileVersionMS);
						minor = LOWORD(ffi->dwFileVersionMS);
					}
				}
			}
		}
	}
#endif
}
static std::string WtoA(const std::wstring& w);
static void CudaToolkitScan(bool& foundWant, bool& foundOther, bool& cudnnOk, std::string& dirs) {
	foundWant = foundOther = cudnnOk = false;
	dirs.clear();
	int major = 0, minor = 0;
	OrtMajorMinor(major, minor);
	bool want13 = (major > 1) || (major == 1 && minor >= 27);
	std::error_code ec;
	std::filesystem::path root(L"C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA");
	if (!std::filesystem::exists(root, ec))return;
	for (auto& entry : std::filesystem::directory_iterator(root, ec)) {
		std::wstring fn = entry.path().filename().wstring();
		if (fn.size() < 3 || fn[0] != L'v')continue;
		bool is12 = fn[1] == L'1' && fn[2] == L'2';
		bool is13 = fn[1] == L'1' && fn[2] == L'3';
		if (!is12 && !is13)continue;
		bool match = (want13 && is13) || (!want13 && is12);
		std::string a = WtoA(fn);
		if (!dirs.empty())dirs += " ";
		dirs += a;
		if (match)foundWant = true;else foundOther = true;
		if (GetFileAttributesW((L"C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\" + fn + L"\\bin\\cudnn64_9.dll").c_str()) != INVALID_FILE_ATTRIBUTES) {
			if (match)cudnnOk = true;
		}
	}
}
static bool CudaAvailable() {
	bool fw = false, fo = false, co = false;
	std::string dirs;
	CudaToolkitScan(fw, fo, co, dirs);
	return (fw || fo) && co;
}
static std::string CudaGpuReport() {
	int major = 0, minor = 0;
	OrtMajorMinor(major, minor);
	bool want13 = (major > 1) || (major == 1 && minor >= 27);
	std::string r = "ORT " + std::string(OrtVersionStr());
	r += want13 ? " | CUDA 13 recommended" : " | CUDA 12 recommended";
	bool foundWant = false, foundOther = false, cudnnOk = false;
	std::string dirs;
	CudaToolkitScan(foundWant, foundOther, cudnnOk, dirs);
	if (!dirs.empty())r += " | installed: " + dirs;
	else r += " | NO CUDA TOOLKIT FOUND";
	r += cudnnOk ? " | cuDNN 9 OK" : " | cuDNN 9 MISSING";
	if ((major > 0) && !want13)r += " | loaded ORT DLL is OLD | update NuGet and do a Clean Rebuild";
	if (foundOther && !foundWant)r += " | minor version differ | GPU tried with safe fallback";
	return r;
}
static Ep PickExecutionProvider(Ort::SessionOptions& opts, std::string& detail) {
	if (g_epBroken.load()) {
		detail = "GPU disabled after invalid output | check models\\sr_gpu.log";
		return EpCpu;
	}
	bool cudaOk = CudaAvailable();
	if (cudaOk) {
		if (AppendEpByName(opts, EpTensorrt)) { detail = "TensorRT execution provider active";return EpTensorrt; }
		if (AppendEpByName(opts, EpCuda)) { detail = "CUDA execution provider active";return EpCuda; }
	}
	if (AppendEpByName(opts, EpDml)) { detail = "DirectML execution provider active";return EpDml; }
	detail = "CPU execution provider active";
	return EpCpu;
}
#endif
struct SdStats {
	double teMs = 0, encMs = 0, unetMs = 0, decMs = 0, upMs = 0;
	float outMean = 0.5f, outStd = 0.0f;
};
#if SR_HAS_ONNX
class SdPipeline {
public:
	Ort::Session LoadBest(const wchar_t* path, bool& gpu, std::string& gpuErr) {
		gpu = false;
		if (m_ep == EpCuda || m_ep == EpTensorrt || m_ep == EpDml) {
			try {
				Ort::SessionOptions gopts;
				gopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				gopts.SetIntraOpNumThreads(0);
				gopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
				AppendEpByName(gopts, m_ep);
				if (m_ep == EpCuda || m_ep == EpTensorrt) { try { gopts.AddConfigEntry("gpu_graph_id", "0"); } catch (...) {} }
				Ort::Session s = Ort::Session(GetOrtEnv(), path, gopts);
				gpu = true;
				return s;
			}
			catch (const std::exception& e) {
				gpuErr = e.what();
				gpu = false;
			}
			try {
				Ort::SessionOptions bopts;
				bopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
				bopts.SetIntraOpNumThreads(0);
				bopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
				bopts.DisableMemPattern();
				try { bopts.AddConfigEntry("dml_disable_metacommands", "1"); }
				catch (...) {}
				AppendEpByName(bopts, m_ep);
				Ort::Session s = Ort::Session(GetOrtEnv(), path, bopts);
				gpu = true;
				return s;
			}
			catch (const std::exception& e) {
				gpuErr = e.what();
				gpu = false;
			}
		}
		Ort::SessionOptions copts;
		copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
		copts.SetIntraOpNumThreads(0);
		copts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
		return Ort::Session(GetOrtEnv(), path, copts);
	}
	bool Init(std::string& err) {
		m_err.clear();
		m_dmlErr.clear();
		if (m_ready)return true;
		sd15::ClipSimpleTokenizer tok;
		std::string terr;
		if (!tok.Load("models\\sd15\\vocab.json", "models\\sd15\\merges.txt", terr)) {
			m_err = "Tokenizer load failed: " + terr;
			return false;
		}
		m_tokenizer = std::move(tok);
		Ort::SessionOptions opts;
		opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
		opts.SetIntraOpNumThreads(0);
		opts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
		m_ep = PickExecutionProvider(opts, m_epErr);
		if (m_ep == EpCuda || m_ep == EpTensorrt) {
			try { opts.SetExecutionMode(ExecutionMode::ORT_PARALLEL); }
			catch (...) {}
		}
		try {
			m_te = LoadBest(L"models\\sd15\\text_encoder.onnx", m_teGpu, m_dmlErr);
			m_unet = LoadBest(L"models\\sd15\\model.onnx", m_unetGpu, m_dmlErr);
			m_vae = LoadBest(L"models\\sd15\\vae_decoder.onnx", m_vaeGpu, m_dmlErr);
			m_vaeE = LoadBest(L"models\\sd15\\vae_encoder.onnx", m_vaeEGpu, m_dmlErr);
		}
		catch (const std::exception& e) {
			m_err = std::string("Could not load the model: ") + e.what();
			return false;
		}
		m_hasFast = false;
		m_hasDs = false;
		if (SdDsReady()) {
			try {
				m_unetDs = LoadBest(L"models\\dreamshaper\\model.onnx", m_dsGpu, m_dmlErr);
				m_hasDs = true;
			}
			catch (...) { m_hasDs = false; }
			if (m_hasDs) {
				m_dsIn0 = OrtSessionInputName(m_unetDs, 0);
				m_dsIn1 = OrtSessionInputName(m_unetDs, 1);
				m_dsIn2 = OrtSessionInputName(m_unetDs, 2);
				m_dsOut = OrtSessionOutputName(m_unetDs, 0);
			}
		}
		{
			try {
				m_unetFast = LoadBest(L"models\\sd15fast\\model.onnx", m_fastGpu, m_dmlErr);
				m_hasFast = true;
			}
			catch (...) { m_hasFast = false; }
		}
		{
			Ort::SessionOptions copts;
			copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			copts.SetIntraOpNumThreads(0);
			try {
				m_vaeCpu = Ort::Session(GetOrtEnv(), L"models\\sd15\\vae_decoder.onnx", copts);
			}
			catch (...) { m_vaeCpu = Ort::Session(nullptr); }
			try {
				m_vaeECpu = Ort::Session(GetOrtEnv(), L"models\\sd15\\vae_encoder.onnx", copts);
			}
			catch (...) { m_vaeECpu = Ort::Session(nullptr); }
			m_vaeCIn = OrtSessionInputName(m_vaeCpu, 0);
			m_vaeCOut = OrtSessionOutputName(m_vaeCpu, 0);
			m_vaeECIn = OrtSessionInputName(m_vaeECpu, 0);
			m_vaeECOut = OrtSessionOutputName(m_vaeECpu, 0);
		}
		m_teIn = OrtSessionInputName(m_te, 0);
		m_teOut = OrtSessionOutputName(m_te, 0);
		for (int i = 0;i < (int)m_unet.GetInputCount();++i)m_unetIns.push_back(OrtSessionInputName(m_unet, i));
		m_unetOut = OrtSessionOutputName(m_unet, 0);
		m_vaeIn = OrtSessionInputName(m_vae, 0);
		m_vaeOut = OrtSessionOutputName(m_vae, 0);
		m_vaeEIn = OrtSessionInputName(m_vaeE, 0);
		m_vaeEOut = OrtSessionOutputName(m_vaeE, 0);
		if (m_hasFast) {
			m_unetFIn0 = OrtSessionInputName(m_unetFast, 0);
			m_unetFIn1 = OrtSessionInputName(m_unetFast, 1);
			m_unetFIn2 = OrtSessionInputName(m_unetFast, 2);
			m_unetFOut = OrtSessionOutputName(m_unetFast, 0);
		}
		m_usingDml = m_teGpu || m_unetGpu || m_vaeGpu || m_vaeEGpu || m_fastGpu || m_dsGpu;
		m_ready = true;
		return true;
	}
	bool IsReady()const { return m_ready; }
	bool UsingDml()const { return m_usingDml; }
	std::string LastErr()const { return m_err; }
	std::string DmlErr()const { return m_dmlErr; }
	bool FastOnGpu()const { return m_fastGpu; }
	bool DsOnGpu()const { return m_dsGpu; }
	bool MainOnGpu()const { return m_unetGpu; }
	bool Generate(const std::string& prompt, const std::string& negPrompt, int steps,
		float cfgScale, int seed, int W, int H,
		const std::vector<uint8_t>* initBgra, int initW, int initH, float denoise,
		std::vector<uint8_t>& outBgra,
		std::atomic<int>* progress, const std::atomic<bool>* cancel,
		double* stepMsOut,
		std::string& errOut) {
		errOut.clear();
		if (!m_ready) { errOut = "Model is not loaded.";return false; }
		if (W % 64 != 0 || H % 64 != 0) { errOut = "Width/Height must be multiples of 64.";return false; }
		if (steps < 1 || steps > 100) { errOut = "Steps out of range.";return false; }
		const int lw = W / 8, lh = H / 8;
		const size_t latentN = (size_t)4 * lh * lw;
		std::vector<int32_t> posIds = m_tokenizer.EncodeFull(prompt, 77);
		std::vector<int32_t> negIds = m_tokenizer.EncodeFull(negPrompt, 77);
		std::vector<float> cond, uncond;
		if (!m_condValid || m_lastPrompt != prompt) {
			EnsureVec(m_cond, (size_t)77 * 768);
			if (!RunTextEncoder(posIds, m_cond, errOut))return false;
			m_lastPrompt = prompt;
			m_condValid = true;
		}
		cond = m_cond;
		EnsureVec(m_uncond, (size_t)77 * 768);
		if (!RunTextEncoder(negIds, m_uncond, errOut))return false;
		uncond = m_uncond;
		std::mt19937 rng((unsigned)seed);
		std::normal_distribution<float> nd(0.0f, 1.0f);
		std::vector<float> latent(latentN);
		for (auto& v : latent)v = nd(rng);
		sd15::DdimScheduler sched;
		std::vector<int> timesteps = sched.Timesteps(steps);
		const int stepDelta = 1000 / steps;
		int startStep = 0;
		std::vector<float> initLatent;
		if (initBgra && !initBgra->empty() && initW > 0 && initH > 0) {
			if (!RunVaeEncode(*initBgra, initW, initH, W, H, initLatent, errOut))return false;
			int stepsToRun = std::max(1, (int)std::lround((double)steps * std::clamp(denoise, 0.05f, 1.0f)));
			startStep = steps - stepsToRun;
			int t0 = timesteps[startStep];
			float acp = sched.AlphaCumprod(t0);
			float sqA = std::sqrt(std::max(acp, 1e-8f));
			float sqB = std::sqrt(std::max(1.0f - acp, 0.0f));
			for (size_t i = 0;i < latentN;++i)
				latent[i] = sqA * initLatent[i] + sqB * latent[i];
		}
		std::vector<float> epsCond(latentN), epsUncond(latentN), epsTotal(latentN), next(latentN);
		auto tStep0 = std::chrono::steady_clock::now();
		int stepCount = 0;
		for (int s = startStep;s < steps;++s) {
			if (cancel && cancel->load()) { errOut = "Cancelled by user.";return false; }
			if (progress)progress->store((int)((double)(s - startStep) * 100.0 / (double)(steps - startStep)));
			int t = timesteps[s];
			if (!RunUnet(latent, t, cond, epsCond, errOut))return false;
			if (!RunUnet(latent, t, uncond, epsUncond, errOut))return false;
			for (size_t i = 0;i < latentN;++i)
				epsTotal[i] = epsUncond[i] + cfgScale * (epsCond[i] - epsUncond[i]);
			sched.Step(latent.data(), epsTotal.data(), next.data(), (int)latentN, t, t - stepDelta);
			latent.swap(next);
			++stepCount;
			auto tNow = std::chrono::steady_clock::now();
			double el = std::chrono::duration<double, std::milli>(tNow - tStep0).count();
			if (stepMsOut && stepCount > 0)*stepMsOut = el / (double)stepCount;
		}
		if (cancel && cancel->load()) { errOut = "Cancelled by user.";return false; }
		if (progress)progress->store(90);
		const float vaeScale = 0.18215f;
		for (auto& v : latent)v /= vaeScale;
		std::vector<float> img;
		if (!RunVaeDecode(latent, W, H, img, errOut))return false;
		outBgra.resize((size_t)W * H * 4);
		for (int y = 0;y < H;++y) {
			for (int x = 0;x < W;++x) {
				size_t idx = (size_t)y * W + x;
				float r = std::clamp((img[0 * (size_t)W * H + idx] + 1.0f) * 0.5f, 0.0f, 1.0f);
				float g = std::clamp((img[1 * (size_t)W * H + idx] + 1.0f) * 0.5f, 0.0f, 1.0f);
				float b = std::clamp((img[2 * (size_t)W * H + idx] + 1.0f) * 0.5f, 0.0f, 1.0f);
				uint8_t* o = &outBgra[idx * 4];
				o[0] = (uint8_t)(b * 255.0f + 0.5f);
				o[1] = (uint8_t)(g * 255.0f + 0.5f);
				o[2] = (uint8_t)(r * 255.0f + 0.5f);
				o[3] = 255;
			}
		}
		if (progress)progress->store(100);
		return true;
	}
	bool HasFast()const { return m_hasFast; }
	void SetFastModel(int m) {
		m_fastIdx = (m == 1 && m_hasDs) ? 1 : 0;
	}
	bool HasDs()const { return m_hasDs; }
	bool HasFastModel(int m)const { return m == 1 ? m_hasDs : m_hasFast; }
	std::string EpName()const {
		if (m_fastGpu) {
			return (m_ep == EpTensorrt) ? "GPU(TensorRT)" : (m_ep == EpCuda) ? "GPU(CUDA)" : "GPU(DirectML)";
		}
		if (m_teGpu || m_unetGpu || m_vaeGpu || m_vaeEGpu || m_dsGpu)return "GPU(DirectML)| UNet: CPU";
		if (m_ep == EpCuda || m_ep == EpTensorrt || m_ep == EpDml)return "CPU(GPU load failed)";
		return "CPU";
	}
	std::string EpDetail()const { return m_epErr; }
	void Warmup() {
		if (m_warmed || !m_hasFast)return;
		m_warmed = true;
		try {
			std::vector<uint8_t> tiny(32 * 32 * 4, 128);
			std::vector<uint8_t> out;
			std::string err;
			SdStats st{};
			GenerateFast("warmup", 1, 1, 32, 32, tiny, 32, 32, 0.5f, out, nullptr, nullptr, &st, err);
		}
		catch (...) {}
	}
	bool GenerateFast(const std::string& prompt, int steps, int seed, int W, int H,
		const std::vector<uint8_t>& initBgra, int initW, int initH, float strength,
		std::vector<uint8_t>& outBgra, std::atomic<int>* progress,
		const std::atomic<bool>* cancel, SdStats* stats, std::string& errOut) {
		errOut.clear();
		if (stats)*stats = SdStats{};
		if (!m_ready || !m_hasFast) { errOut = "Fast engine is not loaded.";return false; }
		if (m_fastIdx == 1 && !m_hasDs) { errOut = "DreamShaper model is not installed.";return false; }
		if (W % 8 != 0 || H % 8 != 0 || W < 8 || H < 8) { errOut = "Size must be a multiple of 8.";return false; }
		steps = std::clamp(steps, 1, 8);
		strength = std::clamp(strength, 0.05f, 1.0f);
		bool hasInit = !initBgra.empty() && initW > 0 && initH > 0;
		const int lw = W / 8, lh = H / 8;
		const size_t latentN = (size_t)4 * lh * lw;
		auto tTe0 = std::chrono::steady_clock::now();
		if (!m_condValid || m_lastPrompt != prompt) {
			std::vector<int32_t> posIds = m_tokenizer.EncodeFull(prompt, 77);
			EnsureVec(m_cond, (size_t)77 * 768);
			if (!RunTextEncoder(posIds, m_cond, errOut))return false;
			m_lastPrompt = prompt;
			m_condValid = true;
		}
		if (stats)stats->teMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tTe0).count();
		sd15::LcmScheduler sched;
		std::vector<int> ts = sched.Timesteps(steps);
		int startIdx = sd15::LcmScheduler::StartIndex(steps, strength);
		auto tEnc0 = std::chrono::steady_clock::now();
		std::mt19937 rng((unsigned)seed);
		std::normal_distribution<float> nd(0.0f, 1.0f);
		EnsureVec(m_latent, latentN);
		EnsureVec(m_noise, latentN);
		EnsureVec(m_epsOut, latentN);
		EnsureVec(m_next, latentN);
		if (hasInit) {
			EnsureVec(m_initLatent, latentN);
			if (!RunVaeEncode(initBgra, initW, initH, W, H, m_initLatent, errOut))return false;
			int tBlend = ts[startIdx];
			float acp = sched.AlphaCumprod(tBlend);
			for (size_t i = 0;i < latentN;++i) {
				float nz = nd(rng);
				m_latent[i] = std::sqrt(acp) * m_initLatent[i] + std::sqrt(1.0f - acp) * nz;
			}
		}
		else {
			startIdx = 0;
			for (size_t i = 0;i < latentN;++i)m_latent[i] = nd(rng);
		}
		if (stats)stats->encMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tEnc0).count();
		auto tUnet0 = std::chrono::steady_clock::now();
		int nDone = 0;
		for (int s = startIdx;s < steps;++s) {
			if (cancel && cancel->load()) { errOut = "Cancelled by user.";return false; }
			int t = ts[s];
			int prevT = (s + 1 < steps) ? ts[s + 1] : -1;
			bool last = (s == steps - 1);
			if (!RunUnetFast(m_latent, t, m_cond, lw, lh, m_epsOut, errOut))return false;
			if (!last)for (size_t i = 0;i < latentN;++i)m_noise[i] = nd(rng);
			sched.Step(m_latent.data(), m_epsOut.data(), m_noise.data(), m_next.data(), (int)latentN, t, prevT, last);
			m_latent.swap(m_next);
			++nDone;
		}
		if (cancel && cancel->load()) { errOut = "Cancelled by user.";return false; }
		if (stats)stats->unetMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tUnet0).count();
		if (progress)progress->store(90);
		const float vaeScale = 0.18215f;
		for (auto& v : m_latent)v /= vaeScale;
		auto tDec0 = std::chrono::steady_clock::now();
		EnsureVec(m_img, (size_t)3 * W * H);
		if (!RunVaeDecode(m_latent, W, H, m_img, errOut))return false;
		if (stats)stats->decMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tDec0).count();
		outBgra.resize((size_t)W * H * 4);
		double sum = 0.0, sum2 = 0.0;
		for (int y = 0;y < H;++y) {
			for (int x = 0;x < W;++x) {
				size_t idx = (size_t)y * W + x;
				float r = std::clamp((m_img[0 * (size_t)W * H + idx] + 1.0f) * 0.5f, 0.0f, 1.0f);
				float g = std::clamp((m_img[1 * (size_t)W * H + idx] + 1.0f) * 0.5f, 0.0f, 1.0f);
				float b = std::clamp((m_img[2 * (size_t)W * H + idx] + 1.0f) * 0.5f, 0.0f, 1.0f);
				uint8_t* o = &outBgra[idx * 4];
				o[0] = (uint8_t)(b * 255.0f + 0.5f);
				o[1] = (uint8_t)(g * 255.0f + 0.5f);
				o[2] = (uint8_t)(r * 255.0f + 0.5f);
				o[3] = 255;
				double v = (double)(r + g + b) / 3.0;
				sum += v;sum2 += v * v;
			}
		}
		if (stats) {
			double n = (double)W * H;
			stats->outMean = (float)(sum / n);
			stats->outStd = (float)std::sqrt(std::max(0.0, sum2 / n - (sum / n) * (sum / n)));
		}
		if (progress)progress->store(100);
		return true;
	}
	static void EnsureVec(std::vector<float>& v, size_t n) {
		if (v.size() != n)v.assign(n, 0.0f);
	}
private:
	bool RunUnetFast(const std::vector<float>& latent, int t, const std::vector<float>& cond,
		int lw, int lh, std::vector<float>& out, std::string& errOut) {
		try {
			Ort::Session& sess = (m_fastIdx == 1) ? m_unetDs : m_unetFast;
			const std::string& in0 = (m_fastIdx == 1) ? m_dsIn0 : m_unetFIn0;
			const std::string& in1 = (m_fastIdx == 1) ? m_dsIn1 : m_unetFIn1;
			const std::string& in2 = (m_fastIdx == 1) ? m_dsIn2 : m_unetFIn2;
			const std::string& outN0 = (m_fastIdx == 1) ? m_dsOut : m_unetFOut;
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			std::array<int64_t, 4> lshape{ 1,4,(int64_t)lh,(int64_t)lw };
			std::array<int64_t, 1> tshape{ 1 };
			std::array<int64_t, 3> eshape{ 1,77,768 };
			const char* inN[3] = { in0.c_str(),in1.c_str(),in2.c_str() };
			const char* outN[] = { outN0.c_str() };
			Ort::Value lten, tten, eten;
			if (SessionInputIsFp16(sess, 0)) {
				m_lat16.resize(latent.size());
				for (size_t li = 0;li < latent.size();++li)m_lat16[li] = F32ToF16(latent[li]);
				lten = MakeFp16Tensor(mi, m_lat16.data(), m_lat16.size(), lshape.data(), lshape.size());
			}
			else {
				lten = Ort::Value::CreateTensor<float>(mi, const_cast<float*>(latent.data()), latent.size(), lshape.data(), lshape.size());
			}
			auto ttyp = sess.GetInputTypeInfo(1).GetTensorTypeAndShapeInfo().GetElementType();
			if (ttyp == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
				int64_t tv = t;
				tten = Ort::Value::CreateTensor<int64_t>(mi, &tv, 1, tshape.data(), tshape.size());
			}
			else if (SessionInputIsFp16(sess, 1)) {
				uint16_t tv = F32ToF16((float)t);
				tten = MakeFp16Tensor(mi, &tv, 1, tshape.data(), tshape.size());
			}
			else {
				float tv = (float)t;
				tten = Ort::Value::CreateTensor<float>(mi, &tv, 1, tshape.data(), tshape.size());
			}
			if (SessionInputIsFp16(sess, 2)) {
				m_cond16.resize(cond.size());
				for (size_t ci = 0;ci < cond.size();++ci)m_cond16[ci] = F32ToF16(cond[ci]);
				eten = MakeFp16Tensor(mi, m_cond16.data(), m_cond16.size(), eshape.data(), eshape.size());
			}
			else {
				eten = Ort::Value::CreateTensor<float>(mi, const_cast<float*>(cond.data()), cond.size(), eshape.data(), eshape.size());
			}
			Ort::Value ins[3] = { std::move(lten),std::move(tten),std::move(eten) };
			auto res = sess.Run(Ort::RunOptions{ nullptr }, inN, ins, 3, outN, 1);
			size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
			if (ValueIsFp16(res[0])) {
				auto* h = res[0].GetTensorMutableData<uint16_t>();
				out = ToF32FromFp16(h, cnt);
			}
			else {
				float* data = res[0].GetTensorMutableData<float>();
				out.assign(data, data + cnt);
			}
			return true;
		}
		catch (const std::exception& e) {
			errOut = std::string("fast engine failed: ") + e.what();
			return false;
		}
	}
	bool RunTextEncoder(const std::vector<int32_t>& ids, std::vector<float>& out, std::string& errOut) {
		try {
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			std::array<int64_t, 2> shape{ 1,77 };
			Ort::Value inV = Ort::Value::CreateTensor<int32_t>(mi, const_cast<int32_t*>(ids.data()), ids.size(), shape.data(), shape.size());
			const char* inN[] = { m_teIn.c_str() };
			const char* outN[] = { m_teOut.c_str() };
			auto res = m_te.Run(Ort::RunOptions{ nullptr }, inN, &inV, 1, outN, 1);
			size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
			if (ValueIsFp16(res[0])) {
				auto* h = res[0].GetTensorMutableData<uint16_t>();
				out = ToF32FromFp16(h, cnt);
			}
			else {
				float* data = res[0].GetTensorMutableData<float>();
				out.assign(data, data + cnt);
			}
			return true;
		}
		catch (const std::exception& e) {
			errOut = std::string("text encoder failed: ") + e.what();
			return false;
		}
	}
	bool RunUnet(const std::vector<float>& latent, int t, const std::vector<float>& cond,
		std::vector<float>& out, std::string& errOut) {
		try {
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			int64_t cells = (int64_t)latent.size() / 4;
			int64_t lh = (int64_t)std::llround(std::sqrt((double)cells));
			int64_t lw = cells / lh;
			while (lh * lw != cells) { ++lh;lw = cells / lh; }
			std::array<int64_t, 4> lshape{ 1,4,lh,lw };
			std::array<int64_t, 1> tshape{ 1 };
			std::array<int64_t, 3> eshape{ 1,77,768 };
			const char* inN[3] = { m_unetIns[0].c_str(),m_unetIns[1].c_str(),m_unetIns[2].c_str() };
			const char* outN[] = { m_unetOut.c_str() };
			Ort::Value lten, tten, eten;
			if (SessionInputIsFp16(m_unet, 0)) {
				auto l16 = ToFp16FromF32(latent.data(), latent.size());
				lten = MakeFp16Tensor(mi, l16.data(), l16.size(), lshape.data(), lshape.size());
			}
			else {
				lten = Ort::Value::CreateTensor<float>(mi, const_cast<float*>(latent.data()), latent.size(), lshape.data(), lshape.size());
			}
			if (SessionInputIsFp16(m_unet, 1)) {
				uint16_t tv = F32ToF16((float)t);
				tten = MakeFp16Tensor(mi, &tv, 1, tshape.data(), tshape.size());
			}
			else {
				int64_t tv = t;
				tten = Ort::Value::CreateTensor<int64_t>(mi, &tv, 1, tshape.data(), tshape.size());
			}
			if (SessionInputIsFp16(m_unet, 2)) {
				auto c16 = ToFp16FromF32(cond.data(), cond.size());
				eten = MakeFp16Tensor(mi, c16.data(), c16.size(), eshape.data(), eshape.size());
			}
			else {
				eten = Ort::Value::CreateTensor<float>(mi, const_cast<float*>(cond.data()), cond.size(), eshape.data(), eshape.size());
			}
			Ort::Value ins[3] = { std::move(lten),std::move(tten),std::move(eten) };
			auto res = m_unet.Run(Ort::RunOptions{ nullptr }, inN, ins, 3, outN, 1);
			size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
			if (ValueIsFp16(res[0])) {
				auto* h = res[0].GetTensorMutableData<uint16_t>();
				out = ToF32FromFp16(h, cnt);
			}
			else {
				float* data = res[0].GetTensorMutableData<float>();
				out.assign(data, data + cnt);
			}
			return true;
		}
		catch (const std::exception& e) {
			errOut = std::string("unet failed: ") + e.what();
			return false;
		}
	}
	bool RunVaeDecode(const std::vector<float>& latent, int W, int H, std::vector<float>& out, std::string& errOut) {
		try {
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			bool useGpu = m_fastGpu && m_vaeGpu && (bool)m_vae;
			Ort::Session& sess = useGpu ? m_vae : m_vaeCpu;
			const std::string& inName = useGpu ? m_vaeIn : m_vaeCIn;
			const std::string& outName = useGpu ? m_vaeOut : m_vaeCOut;
			std::array<int64_t, 4> lshape{ 1,4,(int64_t)H / 8,(int64_t)W / 8 };
			const char* inN[] = { inName.c_str() };
			const char* outN[] = { outName.c_str() };
			Ort::Value lten;
			if (SessionInputIsFp16(sess, 0)) {
				auto l16 = ToFp16FromF32(latent.data(), latent.size());
				lten = MakeFp16Tensor(mi, l16.data(), l16.size(), lshape.data(), lshape.size());
			}
			else {
				lten = Ort::Value::CreateTensor<float>(mi, const_cast<float*>(latent.data()), latent.size(), lshape.data(), lshape.size());
			}
			auto res = sess.Run(Ort::RunOptions{ nullptr }, inN, &lten, 1, outN, 1);
			size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
			if (ValueIsFp16(res[0])) {
				auto* h = res[0].GetTensorMutableData<uint16_t>();
				out = ToF32FromFp16(h, cnt);
			}
			else {
				float* data = res[0].GetTensorMutableData<float>();
				out.assign(data, data + cnt);
			}
			return true;
		}
		catch (const std::exception& e) {
			errOut = std::string("vae decode failed: ") + e.what();
			return false;
		}
	}
	bool RunVaeEncode(const std::vector<uint8_t>& bgra, int srcW, int srcH, int W, int H,
		std::vector<float>& latent, std::string& errOut) {
		try {
			std::vector<float> img((size_t)3 * W * H);
			for (int y = 0;y < H;++y) {
				float sy = ((float)y + 0.5f) * (float)srcH / (float)H - 0.5f;
				sy = std::clamp(sy, 0.0f, (float)(srcH - 1));
				int y0 = (int)sy, y1 = std::min(y0 + 1, srcH - 1);
				float fy = sy - (float)y0;
				for (int x = 0;x < W;++x) {
					float sx = ((float)x + 0.5f) * (float)srcW / (float)W - 0.5f;
					sx = std::clamp(sx, 0.0f, (float)(srcW - 1));
					int x0 = (int)sx, x1 = std::min(x0 + 1, srcW - 1);
					float fx = sx - (float)x0;
					auto px = [&](int xx, int yy)-> std::array<float, 3> {
						const uint8_t* p = &bgra[((size_t)yy * srcW + xx) * 4];
						return { p[2] / 255.0f * 2.0f - 1.0f,p[1] / 255.0f * 2.0f - 1.0f,p[0] / 255.0f * 2.0f - 1.0f };
						};
					auto a = px(x0, y0), b = px(x1, y0), cc = px(x0, y1), d = px(x1, y1);
					size_t idx = (size_t)y * W + x;
					for (int ch = 0;ch < 3;++ch) {
						float top = a[ch] + (b[ch] - a[ch]) * fx;
						float bot = cc[ch] + (d[ch] - cc[ch]) * fx;
						img[ch * (size_t)W * H + idx] = top + (bot - top) * fy;
					}
				}
			}
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			bool useGpu = m_fastGpu && m_vaeEGpu && (bool)m_vaeE;
			Ort::Session& sess = useGpu ? m_vaeE : m_vaeECpu;
			const std::string& inName = useGpu ? m_vaeEIn : m_vaeECIn;
			const std::string& outName = useGpu ? m_vaeEOut : m_vaeECOut;
			std::array<int64_t, 4> ishape{ 1,3,(int64_t)H,(int64_t)W };
			const char* inN[] = { inName.c_str() };
			const char* outN[] = { outName.c_str() };
			Ort::Value iten;
			if (SessionInputIsFp16(sess, 0)) {
				auto i16 = ToFp16FromF32(img.data(), img.size());
				iten = MakeFp16Tensor(mi, i16.data(), i16.size(), ishape.data(), ishape.size());
			}
			else {
				iten = Ort::Value::CreateTensor<float>(mi, img.data(), img.size(), ishape.data(), ishape.size());
			}
			auto res = sess.Run(Ort::RunOptions{ nullptr }, inN, &iten, 1, outN, 1);
			size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
			std::vector<float> raw;
			if (ValueIsFp16(res[0])) {
				auto* h = res[0].GetTensorMutableData<uint16_t>();
				raw = ToF32FromFp16(h, cnt);
			}
			else {
				float* data = res[0].GetTensorMutableData<float>();
				raw.assign(data, data + cnt);
			}
			latent.resize(cnt);
			for (size_t i = 0;i < cnt;++i)latent[i] = raw[i] * 0.18215f;
			return true;
		}
		catch (const std::exception& e) {
			errOut = std::string("vae encode failed: ") + e.what();
			return false;
		}
	}
	sd15::ClipSimpleTokenizer m_tokenizer;
	Ort::Session m_te{ nullptr }, m_unet{ nullptr }, m_vae{ nullptr }, m_vaeE{ nullptr }, m_unetFast{ nullptr }, m_unetDs{ nullptr };
	Ort::Session m_vaeCpu{ nullptr }, m_vaeECpu{ nullptr };
	std::string m_teIn, m_teOut, m_unetOut, m_vaeIn, m_vaeOut, m_vaeEIn, m_vaeEOut, m_unetFIn0, m_unetFIn1, m_unetFIn2, m_unetFOut, m_dsIn0, m_dsIn1, m_dsIn2, m_dsOut;
	std::string m_vaeCIn, m_vaeCOut, m_vaeECIn, m_vaeECOut;
	std::vector<std::string> m_unetIns;
	bool m_ready = false;
	bool m_usingDml = false;
	bool m_hasFast = false;
	bool m_hasDs = false;
	bool m_teGpu = false, m_unetGpu = false, m_vaeGpu = false, m_vaeEGpu = false, m_fastGpu = false, m_dsGpu = false;
	bool m_warmed = false;
	int m_fastIdx = 0;
	Ep m_ep = EpUnknown;
	std::string m_epErr;
	std::vector<float> m_cond, m_uncond, m_initLatent, m_latent, m_noise, m_epsOut, m_next, m_img;
	std::vector<uint16_t> m_lat16, m_cond16;
	std::string m_lastPrompt;
	bool m_condValid = false;
	std::string m_err;
	std::string m_dmlErr;
public:
	void Release() {
		m_te = Ort::Session{ nullptr };
		m_unet = Ort::Session{ nullptr };
		m_vae = Ort::Session{ nullptr };
		m_vaeE = Ort::Session{ nullptr };
		m_unetFast = Ort::Session{ nullptr };
		m_unetDs = Ort::Session{ nullptr };
		m_vaeCpu = Ort::Session{ nullptr };
		m_vaeECpu = Ort::Session{ nullptr };
		m_ready = false;m_hasFast = false;m_hasDs = false;m_warmed = false;m_condValid = false;
		m_teGpu = m_unetGpu = m_vaeGpu = m_vaeEGpu = m_fastGpu = m_dsGpu = false;
		m_unetIns.clear();m_cond.clear();m_uncond.clear();m_initLatent.clear();m_latent.clear();m_noise.clear();m_epsOut.clear();m_next.clear();m_img.clear();
	}
};
#else
class SdPipeline {
public:
	bool Init(std::string& err) { err = "This build was compiled without ONNX Runtime support. Install Microsoft.ML.OnnxRuntime.DirectML(NuGet)and rebuild to enable the AI Image Lab.";return false; }
	bool IsReady()const { return false; }
	bool UsingDml()const { return false; }
	bool FastOnGpu()const { return false; }
	bool DsOnGpu()const { return false; }
	bool MainOnGpu()const { return false; }
	std::string LastErr()const { return m_err; }
	std::string DmlErr()const { return m_err; }
	bool Generate(const std::string&, const std::string&, int, float, int, int, int, const std::vector<uint8_t>*, int, int, float, std::vector<uint8_t>&, std::atomic<int>*, const std::atomic<bool>*, double*, std::string& errOut) {
		errOut = "AI Image Lab requires ONNX Runtime + DirectML support in this build.";
		return false;
	}
	bool HasFast()const { return false; }
	bool HasDs()const { return false; }
	bool HasFastModel(int)const { return false; }
	void SetFastModel(int) {}
	std::string EpName()const { return "unknown"; }
	std::string EpDetail()const { return ""; }
	void Warmup() {}
	bool GenerateFast(const std::string&, int, int, int, int, const std::vector<uint8_t>&, int, int, float, std::vector<uint8_t>&, std::atomic<int>*, const std::atomic<bool>*, SdStats*, std::string& errOut) {
		errOut = "Fast engine requires ONNX Runtime support.";
		return false;
		void Release() {}
	}
private:
	std::string m_err = "not built with ONNX";
};
#endif
static SdProvisioner g_sdProv;
static GenericProvisioner g_upProv;
static bool g_upPending = false;
static SdPipeline g_sdPipeline;
static char g_sdPromptBuf[1024] = "a cyberpunk city at night,neon rain,ultra detailed,volumetric lighting";
static char g_sdNegBuf[512] = "blurry,lowres,bad anatomy,jpeg artifacts,watermark";
static int g_sdSteps = 4;
static float g_sdCfg = 7.5f;
static int g_sdSeed = 42;
static int g_sdW = 384;
static int g_sdH = 384;
static int g_sdSpeedPreset = 1;
static bool g_sdUseScreen = true;
static float g_sdDenoise = 0.45f;
static bool g_sdContinuous = false;
static float g_sdContinuousSecs = 10.0f;
static double g_lastContTime = 0.0;
static bool g_liveEnabled = false;
static int g_liveSteps = 1;
static float g_liveStrength = 0.25f;
static int g_liveRes = 128;
static int g_liveUpMode = 0;
static int g_liveHd = 0;
static std::atomic<bool> g_upGiveUp{ false };
static int g_liveStyle = 0;
static char g_stylePromptBuf[512] = "anime style,vibrant colors,cel shading,clean lineart,studio quality,highly detailed";
static std::atomic<double> g_liveFps{ 0.0 };
static std::atomic<double> g_lastWorkerBeat{ 0.0 };
static std::atomic<double> g_gameFps{ 0.0 };
static std::atomic<bool> g_pubChanged{ true };
static std::atomic<int> g_gameFrames{ 0 };
static std::atomic<double> g_gameT0{ 0.0 };
static std::atomic<bool> g_liveRun{ false };
static std::thread g_liveThread;
static constexpr int kLiveDsW = 352, kLiveDsH = 198;
static constexpr int kRadarDsW = 640, kRadarDsH = 360;
static ComPtr<ID3D11Texture2D> g_liveStaging;
static UINT g_liveStagingW = 0, g_liveStagingH = 0;
static std::mutex g_liveFrameMtx;
static std::vector<uint8_t> g_liveFrameBytes;
static int g_liveFW = 0, g_liveFH = 0;
static std::atomic<int> g_liveFrameVer{ 0 };
static std::atomic<bool> g_livePubBusy{ false };
static std::atomic<int> g_liveOutVer{ 0 };
static std::atomic<int> g_liveShownVer{ 0 };
static std::atomic<bool> g_liveThreadDone{ true };
static std::mutex g_liveOutMtx;
static std::vector<uint8_t> g_liveOuts[3];
static int g_liveOutW = 0, g_liveOutH = 0;
static std::vector<uint8_t> g_livePrevOut;
static int g_liveOutSlot = 0;
static bool g_liveOutReady = false;
struct LiveStats {
	double captureMs = 0, teMs = 0, encMs = 0, unetMs = 0, decMs = 0, upMs = 0, totalMs = 0;
	float outMean = 0.5f, outStd = 0.0f;
	int framesDone = 0, framesSkipped = 0;
};
static LiveStats g_liveStats;
static std::string g_liveLastErr;
static bool g_liveShowRaw = false;
static bool g_liveUpscale = false;
static float g_liveBlend = 0.55f;
static bool g_liveGpuOnly = false;
static std::string LiveEngineName();
struct VisionDetection { float cx, cy, w, h;int cls;float conf; };
struct VisionScene {
	std::vector<VisionDetection> dets;
	bool hasPerson = false, hasVehicle = false;
	float skyFrac = 0.0f, groundFrac = 0.0f, avgDepth = 0.5f;
	uint64_t seq = 0;
};
struct VisionStats {
	double yoloMs = 0, depthMs = 0, samMs = 0, clipMs = 0, totalMs = 0;
	double fps = 0;
	int objects = 0;
};
static const char* kCocoNames[80] = {
 "person","bicycle","car","motorcycle","airplane","bus","train","truck","boat","traffic light",
 "fire hydrant","stop sign","parking meter","bench","bird","cat","dog","horse","sheep","cow",
 "elephant","bear","zebra","giraffe","backpack","umbrella","handbag","tie","suitcase","frisbee",
 "skis","snowboard","sports ball","kite","baseball bat","baseball glove","skateboard","surfboard","tennis racket","bottle",
 "wine glass","cup","fork","knife","spoon","bowl","banana","apple","sandwich","orange",
 "broccoli","carrot","hot dog","pizza","donut","cake","chair","couch","potted plant","bed",
 "dining table","toilet","tv","laptop","mouse","remote","keyboard","cell phone","microwave","oven",
 "toaster","sink","refrigerator","book","clock","vase","scissors","teddy bear","hair drier","toothbrush"
};
static void Letterbox(const std::vector<uint8_t>& frame, int fw, int fh, int sz,
	std::vector<float>& out, float& scale, float& padX, float& padY) {
	scale = (float)sz / (float)std::max(fw, fh);
	int nw = std::max(1, (int)(fw * scale)), nh = std::max(1, (int)(fh * scale));
	padX = (float)(sz - nw) / 2.0f;padY = (float)(sz - nh) / 2.0f;
	out.assign((size_t)3 * sz * sz, 0.0f);
	int offX = (int)(padX + 0.5f), offY = (int)(padY + 0.5f);
	for (int y = 0;y < nh;++y) {
		int sy = std::min(fh - 1, (int)((float)y / scale));
		for (int x = 0;x < nw;++x) {
			int sx = std::min(fw - 1, (int)((float)x / scale));
			const uint8_t* p = &frame[((size_t)sy * fw + sx) * 4];
			size_t idx = (size_t)(y + offY) * sz + (x + offX);
			out[0 * (size_t)sz * sz + idx] = p[2] / 255.0f;
			out[1 * (size_t)sz * sz + idx] = p[1] / 255.0f;
			out[2 * (size_t)sz * sz + idx] = p[0] / 255.0f;
		}
	}
}
static void LetterboxNorm(const std::vector<uint8_t>& frame, int fw, int fh, int sz,
	std::vector<float>& out, float& scale, float& padX, float& padY) {
	Letterbox(frame, fw, fh, sz, out, scale, padX, padY);
	static const float mean[3] = { 0.485f,0.456f,0.406f };
	static const float stdv[3] = { 0.229f,0.224f,0.225f };
	for (int c = 0;c < 3;++c)
		for (size_t i = 0;i < (size_t)sz * sz;++i)
			out[c * (size_t)sz * sz + i] = (out[c * (size_t)sz * sz + i] - mean[c]) / stdv[c];
}
static void YoloDecode(float* raw, size_t cnt, int anchors, int attrs, float scale, float padX, float padY,
	float confThr, std::vector<VisionDetection>& out, float ox, float oy) {
	if (cnt == (size_t)anchors * attrs) {
		for (int a = 0;a < anchors;++a) {
			const float* p = raw + (size_t)a * attrs;
			float best = 0;int bestC = -1;
			for (int c = 4;c < attrs;++c)if (p[c] > best) { best = p[c];bestC = c - 4; }
			if (best > confThr && bestC >= 0) {
				VisionDetection d;d.cx = (p[0] - padX) / scale + ox;d.cy = (p[1] - padY) / scale + oy;
				d.w = p[2] / scale;d.h = p[3] / scale;d.cls = bestC;d.conf = best;
				out.push_back(d);
			}
		}
	}
	else {
		int rows = (int)(cnt / (size_t)anchors);
		for (int a = 0;a < anchors;++a) {
			float best = 0;int bestC = -1;
			for (int c = 4;c < rows;++c) {
				float s = raw[(size_t)c * anchors + a];
				if (s > best) { best = s;bestC = c - 4; }
			}
			if (best > confThr && bestC >= 0) {
				VisionDetection d;d.cx = (raw[a] - padX) / scale + ox;d.cy = (raw[(size_t)anchors + a] - padY) / scale + oy;
				d.w = raw[(size_t)2 * anchors + a] / scale;d.h = raw[(size_t)3 * anchors + a] / scale;
				d.cls = bestC;d.conf = best;
				out.push_back(d);
			}
		}
	}
}
static bool SehCall(bool(*fn)(void*), void* ctx, std::string& err);
class VisionEngine;
class BrainChat;
class VisionEngine {
public:
	bool Init(std::string& err) {
		m_err.clear();
		m_useClip = false;
		{
			sd15::ClipSimpleTokenizer tok;
			std::string terr;
			if (tok.Load("models\\vision\\vocab.json", "models\\vision\\merges.txt", terr)) {
				m_tok = std::move(tok);
				m_useClip = true;
			}
			else {
				m_err = "CLIP tokenizer: " + terr;
			}
		}
		Ort::SessionOptions gopts;
		gopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
		gopts.SetIntraOpNumThreads(0);
		gopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
		bool gpu = false;
		m_epName = "CPU";
		if (!g_epBroken.load()) {
			if (CudaAvailable() && AppendEpByName(gopts, EpCuda)) { gpu = true;m_epName = "GPU(CUDA)"; }
			else if (AppendEpByName(gopts, EpDml)) { gpu = true;m_epName = "GPU(DirectML)"; }
		}
		Ort::SessionOptions copts;
		copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
		copts.SetIntraOpNumThreads(0);
		if (m_useClip) {
			try { m_clip = Ort::Session(GetOrtEnv(), L"models\\vision\\clip_text_encoder.onnx", copts);m_clipIn = OrtSessionInputName(m_clip, 0);m_clipOut = OrtSessionOutputName(m_clip, 0); }
			catch (const std::exception& e) { m_clip = Ort::Session(nullptr);m_useClip = false;m_err += " | CLIP: " + std::string(e.what()); }
		}
		else {
			m_clip = Ort::Session(nullptr);
		}
		try { m_yolo = Ort::Session(GetOrtEnv(), L"models\\vision\\yolo11n.onnx", gopts);m_yoloIn = OrtSessionInputName(m_yolo, 0);m_yoloOut = OrtSessionOutputName(m_yolo, 0);m_yoloPath = L"models\\vision\\yolo11n.onnx"; }
		catch (...) { try { m_yolo = Ort::Session(GetOrtEnv(), L"models\\vision\\yolo11n.onnx", copts);m_yoloIn = OrtSessionInputName(m_yolo, 0);m_yoloOut = OrtSessionOutputName(m_yolo, 0);m_yoloPath = L"models\\vision\\yolo11n.onnx"; } catch (...) { m_yolo = Ort::Session(nullptr); } }
		try { m_depth = Ort::Session(GetOrtEnv(), L"models\\vision\\depth_anything_v2_vits.onnx", gopts);m_depthIn = OrtSessionInputName(m_depth, 0);m_depthOut = OrtSessionOutputName(m_depth, 0); }
		catch (...) { try { m_depth = Ort::Session(GetOrtEnv(), L"models\\vision\\depth_anything_v2_vits.onnx", copts);m_depthIn = OrtSessionInputName(m_depth, 0);m_depthOut = OrtSessionOutputName(m_depth, 0); } catch (...) { m_depth = Ort::Session(nullptr); } }
		m_ready.store(true);
		return true;
	}
	bool IsReady()const { return m_ready.load(); }
	std::string EpName()const { return m_epName; }
	void SetModelUse(bool y, bool s, bool d, bool cl) { m_useYolo = y;m_useSam = s;m_useDepth = d;m_useClip = cl; }
	std::string LastErr()const { return m_err; }
	ShaderStyle PromptToStyle(const std::string& prompt) {
		ShaderStyle st = NeutralStyle();
		std::string lower = prompt;
		std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char ch) {return (char)std::tolower(ch);});
		double t0 = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count();
		std::vector<float> emb;
		bool haveEmb = EmbedText(prompt, emb);
		if (haveEmb && m_conceptEmbs.empty()) { try { ComputeConceptEmbeddings(); } catch (...) {} }
		m_lastUnderstanding.clear();
		if (haveEmb && !m_conceptEmbs.empty()) {
			struct Scored { int idx;float s; };
			std::vector<Scored> scored;
			for (int i = 0;i < kConceptCount;++i) {
				float sim = CosSim(emb, m_conceptEmbs[i]);
				float s = (sim - 0.10f) / (1.0f - 0.10f);
				s = std::max(0.0f, s);
				s += kConcepts[i].prior * 0.02f;
				if (s > 0.03f)scored.push_back({ i,s });
			}
			std::sort(scored.begin(), scored.end(), [](const Scored& a, const Scored& b) {return a.s > b.s;});
			float total = 0;
			int kept = 0;
			for (auto& sc : scored) { if (kept >= 6)break;total += sc.s;++kept; }
			if (total <= 0)total = 1;
			kept = 0;
			int bestTheme = -1;float bestThemeS = 0;
			for (auto& sc : scored) {
				if (kept >= 6)break;
				float w = sc.s / total;
				const ConceptDef& cd = kConcepts[sc.idx];
				st.outlineStrength += cd.d.outlineStrength * w;
				st.bloom += cd.d.bloom * w;
				st.saturation += (cd.d.saturation - 1.0f) * w;
				st.fogDensity += cd.d.fogDensity * w;
				st.toonLevels += cd.d.toonLevels * w;
				st.rimLighting += cd.d.rimLighting * w;
				st.paletteMix += cd.d.paletteMix * w;
				st.edgeSoftness += cd.d.edgeSoftness * w;
				st.contrast += (cd.d.contrast - 1.0f) * w;
				st.vignette += cd.d.vignette * w;
				st.grain += cd.d.grain * w;
				st.chromatic += cd.d.chromatic * w;
				st.neonGlow += cd.d.neonGlow * w;
				st.skyGlow += cd.d.skyGlow * w;
				st.warmCool += cd.d.warmCool * w;
				st.pixelMix += cd.d.pixelMix * w;
				st.rain += cd.d.rain * w;
				st.wind += cd.d.wind * w;
				for (int ci = 0;ci < 3;++ci) {
					st.shadowTint[ci] += cd.d.shadowTint[ci] * w;
					st.highlightTint[ci] += cd.d.highlightTint[ci] * w;
				}
				if (cd.d.themeMode > 0 && sc.s > bestThemeS) { bestThemeS = sc.s;bestTheme = cd.d.themeMode; }
				if (cd.d.tonemapMode > 0 && bestTheme < 0)st.tonemapMode = cd.d.tonemapMode;
				m_lastUnderstanding.push_back({ kConcepts[sc.idx].name,sc.s });
				++kept;
			}
			if (bestTheme >= 0)st.themeMode = bestTheme;
			st.name = m_lastUnderstanding.empty() ? "auto" : m_lastUnderstanding[0].first;
		}
		else {
			int pick = 0;float bestScore = -1;
			for (int i = 0;i < kStylePresetCount;++i) {
				std::string pn = kStylePresets[i].name;std::transform(pn.begin(), pn.end(), pn.begin(), [](unsigned char ch) {return (char)std::tolower(ch);});
				float score = KeywordBoost(lower, pn.c_str()) * 2.0f;
				std::string cp = kStylePresets[i].clipPrompt;std::transform(cp.begin(), cp.end(), cp.begin(), [](unsigned char ch) {return (char)std::tolower(ch);});
				for (const char* kw : { "anime","cel","comic","cyberpunk","neon","watercolor","painterly","oil","pixel","noir","fantasy","cinematic","ps2","retro","ghibli","minecraft","voxel","block","rain","night","sunset" })
					if (cp.find(kw) != std::string::npos)score += KeywordBoost(lower, kw) * 1.5f;
				if (score > bestScore) { bestScore = score;pick = i; }
			}
			st = kStylePresets[pick].s;
			st.name = kStylePresets[pick].name;
		}
		st.outlineStrength = std::clamp(st.outlineStrength, 0.0f, 2.0f);
		st.bloom = std::clamp(st.bloom, 0.0f, 2.0f);
		st.saturation = std::clamp(st.saturation, 0.4f, 2.0f);
		st.fogDensity = std::clamp(st.fogDensity, 0.0f, 1.0f);
		st.toonLevels = std::clamp(st.toonLevels, 0.0f, 8.0f);
		st.rimLighting = std::clamp(st.rimLighting, 0.0f, 1.0f);
		st.paletteMix = std::clamp(st.paletteMix, 0.0f, 1.0f);
		st.edgeSoftness = std::clamp(st.edgeSoftness, 0.0f, 1.0f);
		st.contrast = std::clamp(st.contrast, 0.7f, 1.5f);
		st.vignette = std::clamp(st.vignette, 0.0f, 1.0f);
		st.grain = std::clamp(st.grain, 0.0f, 1.0f);
		st.chromatic = std::clamp(st.chromatic, 0.0f, 1.0f);
		st.neonGlow = std::clamp(st.neonGlow, 0.0f, 2.0f);
		st.skyGlow = std::clamp(st.skyGlow, 0.0f, 2.0f);
		st.warmCool = std::clamp(st.warmCool, -1.0f, 1.0f);
		st.pixelMix = std::clamp(st.pixelMix, 0.0f, 1.0f);
		st.rain = std::clamp(st.rain, 0.0f, 1.0f);
		st.wind = std::clamp(st.wind, 0.0f, 1.0f);
		for (int i = 0;i < 3;++i) {
			st.shadowTint[i] = std::clamp(st.shadowTint[i], 0.0f, 1.0f);
			st.highlightTint[i] = std::clamp(st.highlightTint[i], 0.0f, 1.0f);
		}
		st.outlineEnabled = st.outlineStrength > 0.1f;
		st.fogEnabled = st.fogDensity > 0.02f;
		st.pixelEnabled = st.pixelMix > 0.1f;
		m_lastClipMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count() - t0;
		return st;
	}
	const std::vector<std::pair<std::string, float>>& Understanding()const { return m_lastUnderstanding; }
	void ComputeConceptEmbeddings() {
		if (!m_conceptEmbs.empty())return;
		for (int i = 0;i < kConceptCount;++i) {
			std::vector<float> e;
			if (EmbedText(kConcepts[i].clipText, e))m_conceptEmbs.push_back(std::move(e));
		}
	}
	bool LoadYoloOnly(std::string& err) {
		const SdFileSpec* mspec = &kVisionFiles[g_radarModelIdx];
		if (!ValidateModelFile(mspec->localPath, mspec->minBytes)) { err = "detector model missing";return false; }
		m_useClip = false;m_useSam = false;m_useDepth = false;m_useYolo = true;
		try {
			Ort::SessionOptions gopts;
			gopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			gopts.SetIntraOpNumThreads(0);
			gopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
			bool got = false;
			if (!g_epBroken.load()) {
				if (CudaAvailable() && AppendEpByName(gopts, EpCuda)) { got = true;m_epName = "GPU(CUDA)"; }
				else if (AppendEpByName(gopts, EpDml)) { got = true;m_epName = "GPU(DirectML)"; }
			}
			Ort::Session s(GetOrtEnv(), mspec->localPath, gopts);
			m_yolo = std::move(s);
			m_yoloIn = OrtSessionInputName(m_yolo, 0);
			m_yoloOut = OrtSessionOutputName(m_yolo, 0);
			m_yoloPath = mspec->localPath;
			if (!got)m_epName = "CPU";
			m_ready.store(true);
			return true;
		}
		catch (const std::exception& e) {
			try {
				Ort::SessionOptions copts;
				copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				copts.SetIntraOpNumThreads(0);
				Ort::Session s(GetOrtEnv(), mspec->localPath, copts);
				m_yolo = std::move(s);
				m_yoloIn = OrtSessionInputName(m_yolo, 0);
				m_yoloOut = OrtSessionOutputName(m_yolo, 0);
				m_yoloPath = mspec->localPath;
				m_epName = "CPU";
				m_ready.store(true);
				return true;
			}
			catch (const std::exception& e2) {
				err = std::string("detector load: ") + e2.what();
				return false;
			}
		}
	}
	void ReloadYoloCpu() {
		if (!m_yolo)return;
		try {
			Ort::SessionOptions copts;
			copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			copts.SetIntraOpNumThreads(0);
			Ort::Session s(GetOrtEnv(), L"models\\vision\\yolo11n.onnx", copts);
			m_yolo = std::move(s);
			m_yoloIn = OrtSessionInputName(m_yolo, 0);
			m_yoloOut = OrtSessionOutputName(m_yolo, 0);
			m_epName = "CPU";
		}
		catch (...) {}
	}
	bool SetYoloModel(const wchar_t* path) {
		try {
			Ort::SessionOptions gopts;
			gopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			gopts.SetIntraOpNumThreads(0);
			gopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
			bool got = false;
			if (!g_epBroken.load()) {
				if (CudaAvailable() && AppendEpByName(gopts, EpCuda)) { got = true;m_epName = "GPU(CUDA)"; }
				else if (AppendEpByName(gopts, EpDml)) { got = true;m_epName = "GPU(DirectML)"; }
			}
			Ort::Session s(GetOrtEnv(), path, gopts);
			m_yolo = std::move(s);
			m_yoloIn = OrtSessionInputName(m_yolo, 0);
			m_yoloOut = OrtSessionOutputName(m_yolo, 0);
			m_yoloPath = path;
			if (!got)m_epName = "CPU";
			return true;
		}
		catch (...) {
			try {
				Ort::SessionOptions copts;
				copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				copts.SetIntraOpNumThreads(0);
				Ort::Session s(GetOrtEnv(), path, copts);
				m_yolo = std::move(s);
				m_yoloIn = OrtSessionInputName(m_yolo, 0);
				m_yoloOut = OrtSessionOutputName(m_yolo, 0);
				m_yoloPath = path;
				m_epName = "CPU";
				return true;
			}
			catch (...) { return false; }
		}
	}
	const std::wstring& YoloPath()const { return m_yoloPath; }
	bool RunYoloImpl(const std::vector<uint8_t>& frame, int fw, int fh, std::vector<VisionDetection>& out,
		float confThr, float iouThr, int sz, bool zoom, float focusX = 0.5f, float focusY = 0.5f) {
		if (!m_yolo || !m_useYolo)return false;
		const int attrs = 84;
		std::vector<VisionDetection> cands;
		auto runPass = [&](const std::vector<uint8_t>& img, int iw, int ih, float ox, float oy)-> bool {
			std::vector<float> input;
			float scale, padX, padY;
			Letterbox(img, iw, ih, sz, input, scale, padX, padY);
			try {
				Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
				std::array<int64_t, 4> shape{ 1,3,sz,sz };
				Ort::Value inT = Ort::Value::CreateTensor<float>(mi, input.data(), input.size(), shape.data(), shape.size());
				const char* inN[] = { m_yoloIn.c_str() };
				const char* outN[] = { m_yoloOut.c_str() };
				auto res = m_yolo.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN, 1);
				float* raw = res[0].GetTensorMutableData<float>();
				size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
				for (size_t i = 0;i < cnt;++i) {
					if (!std::isfinite(raw[i])) { g_epBroken.store(true);return false; }
				}
				int anchors = (sz / 8) * (sz / 8) + (sz / 16) * (sz / 16) + (sz / 32) * (sz / 32);
				YoloDecode(raw, cnt, anchors, attrs, scale, padX, padY, confThr, cands, ox, oy);
				return true;
			}
			catch (...) { return false; }
			};
		if (!runPass(frame, fw, fh, 0.0f, 0.0f))return false;
		if (zoom && fw >= 128 && fh >= 72) {
			int cw = fw / 2, ch = fh / 2;
			int ox = std::clamp((int)(focusX * (float)fw) - cw / 2, 0, fw - cw);
			int oy = std::clamp((int)(focusY * (float)fh) - ch / 2, 0, fh - ch);
			std::vector<uint8_t> crop((size_t)cw * ch * 4);
			for (int y = 0;y < ch;++y)
				memcpy(&crop[(size_t)y * cw * 4], &frame[((size_t)(oy + y) * fw + ox) * 4], (size_t)cw * 4);
			if (!runPass(crop, cw, ch, (float)ox, (float)oy))return false;
		}
		std::sort(cands.begin(), cands.end(), [](const VisionDetection& a, const VisionDetection& b) {return a.conf > b.conf;});
		for (auto& d : cands) {
			bool keep = true;
			for (auto& k : out) {
				float ix = std::max(0.0f, std::min(d.cx + d.w / 2, k.cx + k.w / 2) - std::max(d.cx - d.w / 2, k.cx - k.w / 2));
				float iy = std::max(0.0f, std::min(d.cy + d.h / 2, k.cy + k.h / 2) - std::max(d.cy - d.h / 2, k.cy - k.h / 2));
				float inter = ix * iy;
				float uni = d.w * d.h + k.w * k.h - inter;
				if (inter / std::max(uni, 1e-6f) > iouThr) { keep = false;break; }
			}
			if (keep)out.push_back(d);
			if (out.size() >= 40)break;
		}
		return true;
	}
	bool RunYolo(const std::vector<uint8_t>& frame, int fw, int fh, std::vector<VisionDetection>& out,
		float confThr = 0.35f, float iouThr = 0.30f, int sz = 640, bool zoom = false,
		float focusX = 0.5f, float focusY = 0.5f) {
		if (!m_yolo || !m_useYolo)return false;
		if (g_epBroken.load())ReloadYoloCpu();
		return RunYoloImpl(frame, fw, fh, out, confThr, iouThr, sz, zoom, focusX, focusY);
	}
	bool RunDepth(const std::vector<uint8_t>& frame, int fw, int fh, std::vector<float>& depth, int& sz, float& skyFrac, float& groundFrac, float& avg) {
		if (!m_depth || !m_useDepth)return false;
		sz = 224;
		std::vector<float> input;
		float scale, padX, padY;
		LetterboxNorm(frame, fw, fh, sz, input, scale, padX, padY);
		try {
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			std::array<int64_t, 4> shape{ 1,3,sz,sz };
			Ort::Value inT = Ort::Value::CreateTensor<float>(mi, input.data(), input.size(), shape.data(), shape.size());
			const char* inN[] = { m_depthIn.c_str() };
			const char* outN[] = { m_depthOut.c_str() };
			auto res = m_depth.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN, 1);
			float* raw = res[0].GetTensorMutableData<float>();
			size_t cnt = (size_t)sz * sz;
			float mn = *std::min_element(raw, raw + cnt), mx = *std::max_element(raw, raw + cnt);
			float rng = std::max(1e-6f, mx - mn);
			depth.resize(cnt);
			double skySum = 0, skyN = 0, grSum = 0, grN = 0, avgSum = 0;
			for (int y = 0;y < sz;++y)
				for (int x = 0;x < sz;++x) {
					float v = (raw[(size_t)y * sz + x] - mn) / rng;
					depth[(size_t)y * sz + x] = v;
					avgSum += v;
					if (y < sz / 3 && v > 0.8f) { skySum += 1; }
					if (y > sz * 3 / 4 && v < 0.55f) { grSum += 1; }
				}
			skyN = (double)(sz / 3) * sz;grN = (double)(sz / 4) * sz;
			skyFrac = (float)(skySum / skyN);
			groundFrac = (float)(grSum / grN);
			avg = (float)(avgSum / cnt);
			return true;
		}
		catch (...) { return false; }
	}
	bool RunSam(const std::vector<uint8_t>& frame, int fw, int fh, float bx, float by, float bw, float bh,
		std::vector<uint8_t>& mask, int& mw, int& mh) {
		if (!m_useSam)return false;
		if (!m_samInit) {
			m_samInit = true;
			Ort::SessionOptions gopts;
			gopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			gopts.SetIntraOpNumThreads(0);
			gopts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
			AppendEpByName(gopts, EpDml);
			try {
				m_samEnc = Ort::Session(GetOrtEnv(), L"models\\vision\\sam2_vision_encoder.onnx", gopts);
				m_samDec = Ort::Session(GetOrtEnv(), L"models\\vision\\sam2_mask_decoder.onnx", gopts);
			}
			catch (...) {
				try {
					Ort::SessionOptions copts;copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);copts.SetIntraOpNumThreads(0);
					m_samEnc = Ort::Session(GetOrtEnv(), L"models\\vision\\sam2_vision_encoder.onnx", copts);
					m_samDec = Ort::Session(GetOrtEnv(), L"models\\vision\\sam2_mask_decoder.onnx", copts);
				}
				catch (...) { return false; }
			}
			m_samEncIn = OrtSessionInputName(m_samEnc, 0);
			m_samDecPts = OrtSessionInputName(m_samDec, 0);
			m_samDecLbl = OrtSessionInputName(m_samDec, 1);
			m_samDecBox = OrtSessionInputName(m_samDec, 2);
			m_samDecE0 = OrtSessionInputName(m_samDec, 3);
			m_samDecE1 = OrtSessionInputName(m_samDec, 4);
			m_samDecE2 = OrtSessionInputName(m_samDec, 5);
		}
		if (!m_samEnc)return false;
		const int sz = 1024;
		std::vector<float> input;
		float scale, padX, padY;
		LetterboxNorm(frame, fw, fh, sz, input, scale, padX, padY);
		try {
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			std::array<int64_t, 4> shape{ 1,3,sz,sz };
			Ort::Value inT = Ort::Value::CreateTensor<float>(mi, input.data(), input.size(), shape.data(), shape.size());
			const char* inN[] = { m_samEncIn.c_str() };
			const char* outN0[] = { "image_embeddings.0" }, * outN1[] = { "image_embeddings.1" }, * outN2[] = { "image_embeddings.2" };
			auto e0 = m_samEnc.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN0, 1);
			auto e1 = m_samEnc.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN1, 1);
			auto e2 = m_samEnc.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN2, 1);
			float x1 = std::max(0.0f, (bx - bw / 2) / scale + padX), y1 = std::max(0.0f, (by - bh / 2) / scale + padY);
			float x2 = std::min((float)sz, (bx + bw / 2) / scale + padX), y2 = std::min((float)sz, (by + bh / 2) / scale + padY);
			if (x2 - x1 < 8) { x1 = std::max(0.0f, x1 - 8);x2 = std::min((float)sz, x2 + 8); }
			if (y2 - y1 < 8) { y1 = std::max(0.0f, y1 - 8);y2 = std::min((float)sz, y2 + 8); }
			float pts[2] = { (x1 + x2) / 2,(y1 + y2) / 2 };
			int64_t lbl[1] = { 2 };
			float box[4] = { x1,y1,x2,y2 };
			std::array<int64_t, 4> pshape{ 1,1,1,2 };
			std::array<int64_t, 3> lshape{ 1,1,1 };
			std::array<int64_t, 3> bshape{ 1,1,4 };
			std::array<int64_t, 4> e0shape{ 1,32,256,256 }, e1shape{ 1,64,128,128 }, e2shape{ 1,256,64,64 };
			Ort::Value pT = Ort::Value::CreateTensor<float>(mi, pts, 2, pshape.data(), pshape.size());
			Ort::Value lT = Ort::Value::CreateTensor<int64_t>(mi, lbl, 1, lshape.data(), lshape.size());
			Ort::Value bT = Ort::Value::CreateTensor<float>(mi, box, 4, bshape.data(), bshape.size());
			Ort::Value e0T = Ort::Value::CreateTensor<float>(mi, e0[0].GetTensorMutableData<float>(), 32 * 256 * 256, e0shape.data(), e0shape.size());
			Ort::Value e1T = Ort::Value::CreateTensor<float>(mi, e1[0].GetTensorMutableData<float>(), 64 * 128 * 128, e1shape.data(), e1shape.size());
			Ort::Value e2T = Ort::Value::CreateTensor<float>(mi, e2[0].GetTensorMutableData<float>(), 256 * 64 * 64, e2shape.data(), e2shape.size());
			const char* dinN[] = { m_samDecPts.c_str(),m_samDecLbl.c_str(),m_samDecBox.c_str(),m_samDecE0.c_str(),m_samDecE1.c_str(),m_samDecE2.c_str() };
			Ort::Value dins[] = { std::move(pT),std::move(lT),std::move(bT),std::move(e0T),std::move(e1T),std::move(e2T) };
			const char* doutN[] = { "pred_masks" };
			auto dres = m_samDec.Run(Ort::RunOptions{ nullptr }, dinN, dins, 6, doutN, 1);
			float* masks = dres[0].GetTensorMutableData<float>();
			mw = 64;mh = 36;
			mask.assign((size_t)mw * mh, 0);
			const int mm = 256;
			for (int y = 0;y < mh;++y)
				for (int x = 0;x < mw;++x) {
					float uvx = ((float)x + 0.5f) / mw, uvy = ((float)y + 0.5f) / mh;
					float fx = uvx * fw, fy = uvy * fh;
					float mx = (fx / scale + padX) / sz * mm, my = (fy / scale + padY) / sz * mm;
					int ix = std::clamp((int)mx, 0, mm - 1), iy = std::clamp((int)my, 0, mm - 1);
					float v = masks[((size_t)iy * mm + ix) * 3];
					mask[(size_t)y * mw + x] = (uint8_t)(std::clamp(v, 0.0f, 1.0f) * 255.0f);
				}
			return true;
		}
		catch (...) { return false; }
	}
private:
	bool EmbedText(const std::string& prompt, std::vector<float>& out) {
		if (!m_clip || !m_useClip)return false;
		try {
			auto ids = m_tok.EncodeFull(prompt, 77);
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			std::array<int64_t, 2> shape{ 1,77 };
			Ort::Value inT = Ort::Value::CreateTensor<int32_t>(mi, ids.data(), ids.size(), shape.data(), shape.size());
			const char* inN[] = { m_clipIn.c_str() };
			const char* outN[] = { m_clipOut.c_str() };
			auto res = m_clip.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN, 1);
			size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
			if (ValueIsFp16(res[0])) { auto* h = res[0].GetTensorMutableData<uint16_t>();out = ToF32FromFp16(h, cnt); }
			else { float* d = res[0].GetTensorMutableData<float>();out.assign(d, d + cnt); }
			return true;
		}
		catch (...) { return false; }
	}
	static float CosSim(const std::vector<float>& a, const std::vector<float>& b) {
		float dot = 0, na = 0, nb = 0;
		for (size_t i = 0;i < a.size();++i) { dot += a[i] * b[i];na += a[i] * a[i];nb += b[i] * b[i]; }
		return dot / (std::sqrt(na) * std::sqrt(nb) + 1e-8f);
	}
	ShaderStyle BlendPresets(int a, int b, float wa) {
		ShaderStyle st = kStylePresets[a].s;
		const ShaderStyle& sb = kStylePresets[b].s;
		float wb = 1.0f - wa;
		st.outlineStrength = st.outlineStrength * wa + sb.outlineStrength * wb;
		st.bloom = st.bloom * wa + sb.bloom * wb;
		st.saturation = st.saturation * wa + sb.saturation * wb;
		st.fogDensity = st.fogDensity * wa + sb.fogDensity * wb;
		st.toonLevels = (st.toonLevels > 0 && sb.toonLevels > 0) ? (st.toonLevels * wa + sb.toonLevels * wb) : std::max(st.toonLevels, sb.toonLevels);
		st.rimLighting = st.rimLighting * wa + sb.rimLighting * wb;
		st.paletteMix = st.paletteMix * wa + sb.paletteMix * wb;
		st.edgeSoftness = st.edgeSoftness * wa + sb.edgeSoftness * wb;
		st.contrast = st.contrast * wa + sb.contrast * wb;
		st.vignette = st.vignette * wa + sb.vignette * wb;
		st.grain = st.grain * wa + sb.grain * wb;
		st.chromatic = st.chromatic * wa + sb.chromatic * wb;
		st.neonGlow = st.neonGlow * wa + sb.neonGlow * wb;
		st.skyGlow = st.skyGlow * wa + sb.skyGlow * wb;
		st.warmCool = st.warmCool * wa + sb.warmCool * wb;
		st.pixelMix = st.pixelMix * wa + sb.pixelMix * wb;
		for (int i = 0;i < 3;++i) { st.shadowTint[i] = st.shadowTint[i] * wa + sb.shadowTint[i] * wb;st.highlightTint[i] = st.highlightTint[i] * wa + sb.highlightTint[i] * wb; }
		st.themeMode = wa > 0.5f ? st.themeMode : sb.themeMode;
		st.tonemapMode = wa > 0.5f ? st.tonemapMode : sb.tonemapMode;
		st.outlineEnabled = st.outlineStrength > 0.1f;
		st.fogEnabled = st.fogDensity > 0.02f;
		st.pixelEnabled = st.pixelMix > 0.1f;
		st.name = kStylePresets[a].name;
		return st;
	}
	sd15::ClipSimpleTokenizer m_tok;
	Ort::Session m_clip{ nullptr }, m_yolo{ nullptr }, m_depth{ nullptr };
	Ort::Session m_samEnc{ nullptr }, m_samDec{ nullptr };
	std::string m_clipIn, m_clipOut, m_yoloIn, m_yoloOut, m_depthIn, m_depthOut;
	std::string m_samEncIn, m_samDecPts, m_samDecLbl, m_samDecBox, m_samDecE0, m_samDecE1, m_samDecE2;
	std::atomic<bool> m_ready{ false };
	bool m_samInit = false;
	bool m_useYolo = true, m_useSam = true, m_useDepth = true, m_useClip = true;
	std::string m_epName = "CPU";
	std::wstring m_yoloPath;
	std::string m_err;
	std::vector<std::vector<float>> m_presetEmbs;
	std::vector<std::vector<float>> m_conceptEmbs;
	std::vector<std::pair<std::string, float>> m_lastUnderstanding;
	double m_lastClipMs = 0;
public:
	void ComputePresetEmbeddings() {
		m_presetEmbs.clear();
		for (int i = 0;i < kStylePresetCount;++i) {
			std::vector<float> e;
			if (EmbedText(kStylePresets[i].clipPrompt, e))m_presetEmbs.push_back(std::move(e));
		}
	}
	double LastClipMs()const { return m_lastClipMs; }
};
static VisionEngine g_vision;
static bool g_visionEnabled = false;
static bool g_modelYolo = true;
static bool g_modelSam = true;
static bool g_modelDepth = true;
static bool g_modelClip = true;
static std::atomic<bool> g_visionRun{ false };
static std::thread g_visionThread;
static std::atomic<bool> g_visionThreadDone{ true };
static std::atomic<double> g_lastVisionBeat{ 0.0 };
static std::mutex g_visionMtx;
static ShaderStyle g_visionStyle;
static VisionScene g_visionScene;
static VisionStats g_visionStats;
static std::string g_visionLastErr;
static std::string g_visionPrompt = "Anime";
static std::atomic<bool> g_visionPromptDirty{ true };
static std::atomic<int> g_visionPresetSel{ -1 };
static ShaderStyle g_activeStyle;
static bool g_gpuDrawMode = false;
static int g_drawPreset = 0;
static std::vector<std::pair<std::string, float>> g_visionUnderstanding;
static std::mutex g_visUndMtx;
static float g_styleIntensity = 1.0f;
static std::mutex g_maskPubMtx;
static std::vector<uint8_t> g_maskPub;
static int g_maskPubW = 0, g_maskPubH = 0;
static std::atomic<bool> g_maskPubDirty{ false };
static SdProvisioner g_visionProv;
struct StylePreset { const char* name;const char* prompt;float strength;int steps; };
static const StylePreset kStyles[] = {
{"Anime","anime style,vibrant colors,cel shading,clean lineart,studio quality,highly detailed",0.55f,2},
{"Manga Sketch","black and white manga sketch,bold ink lineart,screentone shading,high contrast",0.65f,2},
{"Watercolor","watercolor painting,soft color washes,paper texture,delicate brushwork",0.60f,2},
{"Oil Painting","oil painting,impasto brushstrokes,rich saturated colors,canvas texture",0.60f,4},
{"Neon Cyberpunk","neon cyberpunk,glowing neon lights,rainy night street,cinematic",0.60f,4},
{"Ghibli","studio ghibli style,warm colors,soft lighting,detailed whimsical background",0.55f,4},
{"3D Render","high quality 3d render,octane render,detailed materials,soft studio lighting",0.45f,4},
};
static constexpr int kStyleCount = (int)(sizeof(kStyles) / sizeof(kStyles[0]));
enum class SdGenState { Idle, Downloading, DownloadFailed, Ready, Generating, Done, Failed };
static SdGenState g_sdGenState = SdGenState::Idle;
static std::atomic<int> g_sdProgress{ 0 };
static std::atomic<bool> g_sdCancel{ false };
static std::atomic<double> g_sdStepMs{ 0.0 };
static std::string g_sdStatus = "AI Image Lab | download the model once(~2.75 GB)to generate.";
static std::string g_sdLastError;
static std::string g_sdEpInfo = "Engine not loaded";
static std::thread g_sdThread;
static std::vector<uint8_t> g_generatedRgba;
static int g_generatedW = 0, g_generatedH = 0;
static ID3D11ShaderResourceView* g_generatedSrv = nullptr;
static std::mutex g_genMtx;
static std::vector<uint8_t> g_resultRgba;
static int g_resultW = 0, g_resultH = 0;
static bool g_resultReady = false;
static std::mutex g_resultMtx;
static bool g_sdAutoApply = true;
static bool g_sdApplied = false;
static void UpdateGeneratedTexture(ID3D11Device* dev, ID3D11DeviceContext* ctx, const std::vector<uint8_t>& rgba, int w, int h) {
	if (!dev || !ctx || rgba.empty() || w <= 0 || h <= 0)return;
	std::lock_guard<std::mutex> lk(g_genMtx);
	if (g_generatedSrv) { g_generatedSrv->Release();g_generatedSrv = nullptr; }
	D3D11_TEXTURE2D_DESC td{};
	td.Width = (UINT)w;td.Height = (UINT)h;td.MipLevels = 1;td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA srd{};srd.pSysMem = rgba.data();srd.SysMemPitch = w * 4;
	ID3D11Texture2D* tex = nullptr;
	if (FAILED(dev->CreateTexture2D(&td, &srd, &tex)))return;
	HRESULT hr = dev->CreateShaderResourceView(tex, nullptr, &g_generatedSrv);
	tex->Release();
	if (FAILED(hr))return;
	g_generatedW = w;g_generatedH = h;g_generatedRgba = rgba;
}
static bool SdFilesReady() {
	for (int i = 0;i < kSdFileCount;++i)
		if (!ValidateModelFile(kSdFiles[i].localPath, kSdFiles[i].minBytes))return false;
	return true;
}
static bool SdFastReady() {
	static const int fastIdx[] = { 0,1,2,3,4,5,6,7,8 };
	for (int f : fastIdx)
		if (!ValidateModelFile(kSdFiles[f].localPath, kSdFiles[f].minBytes))return false;
	return true;
}
static SdFileSpec g_sdNeed[9];
static int g_sdNeedN = 0;
static int SdFastMissingSpecs(SdFileSpec out[9]) {
	static const int fastIdx[] = { 0,1,2,3,4,5,6,7,8 };
	int n = 0;
	for (int f : fastIdx)
		if (!ValidateModelFile(kSdFiles[f].localPath, kSdFiles[f].minBytes))out[n++] = kSdFiles[f];
	return n;
}
static std::string SdFastMissing() {
	static const int fastIdx[] = { 0,1,2,3,4,5,6,7,8 };
	std::string out;
	for (int f : fastIdx) {
		if (!ValidateModelFile(kSdFiles[f].localPath, kSdFiles[f].minBytes)) {
			if (!out.empty())out += ",";
			out += kSdFiles[f].label;
		}
	}
	return out;
}
static void SdGenerateWorker(int steps, float cfg, int seed, int W, int H,
	std::string prompt, std::string neg,
	std::vector<uint8_t> initFrame, int initW, int initH, float denoise) {
	std::string err;
	if (!g_sdPipeline.IsReady()) {
		g_sdStatus = "Loading engine...";
		if (!g_sdPipeline.Init(err)) {
			g_sdStatus = "Could not load the model: " + g_sdPipeline.LastErr();
			g_sdGenState = SdGenState::Failed;
			return;
		}
		g_sdEpInfo = g_sdPipeline.EpName() + (g_sdPipeline.EpDetail().empty() ? std::string() : " | " + g_sdPipeline.EpDetail());
	}
	bool useScreen = !initFrame.empty() && initW > 0 && initH > 0;
	std::vector<uint8_t> out;
	double stepMs = 0.0;
	if (!g_sdPipeline.Generate(prompt, neg, steps, cfg, seed, W, H,
		useScreen ? &initFrame : nullptr, initW, initH, denoise,
		out, &g_sdProgress, &g_sdCancel, &stepMs, err)) {
		g_sdStatus = err;
		g_sdGenState = SdGenState::Failed;
		return;
	}
	g_sdStepMs.store(stepMs);
	{
		std::lock_guard<std::mutex> lk(g_resultMtx);
		g_resultRgba = std::move(out);
		g_resultW = W;
		g_resultH = H;
		g_resultReady = true;
	}
	g_sdGenState = SdGenState::Done;
	g_sdStatus = "Done | " + std::to_string(W) + "x" + std::to_string(H) + " @ " + std::to_string(steps) + " steps,seed " + std::to_string(seed);
}
static void SdFastWorker(int steps, int seed, int W, int H, std::string prompt,
	std::vector<uint8_t> initFrame, int initW, int initH, float strength) {
	std::string err;
	if (!g_sdPipeline.IsReady()) {
		g_sdStatus = "Loading engine...";
		if (!g_sdPipeline.Init(err)) {
			g_sdStatus = "Could not load the model: " + g_sdPipeline.LastErr();
			g_sdGenState = SdGenState::Failed;
			return;
		}
		g_sdEpInfo = g_sdPipeline.EpName();
	}
	g_sdPipeline.SetFastModel(g_liveModel);
	if (g_liveModel == 1 && !g_sdPipeline.HasDs()) {
		g_sdStatus = "DreamShaper V7 not installed | pick the Fast model or download it.";
		g_sdGenState = SdGenState::Failed;
		return;
	}
	std::vector<uint8_t> out;
	SdStats st{};
	if (!g_sdPipeline.GenerateFast(prompt, steps, seed, W, H, initFrame, initW, initH,
		strength, out, &g_sdProgress, &g_sdCancel, &st, err)) {
		g_sdStatus = err + "[" + g_sdPipeline.EpName() + "]";
		g_sdGenState = SdGenState::Failed;
		return;
	}
	UpdateGeneratedTexture(g_dev, g_ctx, out, W, H);
	g_sdStepMs.store(st.unetMs);
	g_sdGenState = SdGenState::Done;
	g_sdStatus = "Done | " + std::to_string(W) + "x" + std::to_string(H) + " @ " + std::to_string(steps) + " steps," + g_sdPipeline.EpName();
}
static void SdStartFast(std::vector<uint8_t> frame, int fw, int fh) {
	if (g_sdThread.joinable())g_sdThread.detach();
	g_sdCancel.store(false);
	g_sdProgress.store(0);
	g_sdStepMs.store(0.0);
	g_sdGenState = SdGenState::Generating;
	g_sdStatus = "Generating image...";
	int steps = std::clamp(g_sdSteps, 1, 8);
	int W = std::clamp((g_sdW / 64) * 64, 64, 1024);
	int H = std::clamp((g_sdH / 64) * 64, 64, 1024);
	int seed = g_sdSeed;
	std::string prompt(g_sdPromptBuf);
	float strength = std::clamp(g_sdDenoise, 0.05f, 1.0f);
	g_sdThread = std::thread([=] {SdFastWorker(steps, seed, W, H, prompt, std::move(frame), fw, fh, strength);});
}
static void SdStartGeneration(std::vector<uint8_t> initFrame, int initW, int initH) {
	if (g_sdThread.joinable())g_sdThread.detach();
	g_sdCancel.store(false);
	g_sdProgress.store(0);
	g_sdStepMs.store(0.0);
	g_sdGenState = SdGenState::Generating;
	if (!initFrame.empty()) {
		g_sdStatus = g_sdContinuous ? "Re-imagining your screen..." : "Re-imagining current screen...";
	}
	else {
		g_sdStatus = "Generating image...";
	}
	int steps = std::clamp(g_sdSteps, 1, 100);
	float cfg = std::clamp(g_sdCfg, 1.0f, 20.0f);
	int W = std::clamp((g_sdW / 64) * 64, 64, 1024);
	int H = std::clamp((g_sdH / 64) * 64, 64, 1024);
	int seed = g_sdSeed;
	std::string prompt(g_sdPromptBuf);
	std::string neg(g_sdNegBuf);
	float denoise = std::clamp(g_sdDenoise, 0.05f, 1.0f);
	g_sdThread = std::thread([=] {SdGenerateWorker(steps, cfg, seed, W, H, prompt, neg, std::move(initFrame), initW, initH, denoise);});
}
static bool SaveRgbaAsPngWIC(const std::wstring& path, const std::vector<uint8_t>& rgbaBGRA, int w, int h) {
	if (rgbaBGRA.empty() || w <= 0 || h <= 0)return false;
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	ComPtr<IWICImagingFactory> factory;
	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
	if (FAILED(hr))return false;
	ComPtr<IWICStream> stream;factory->CreateStream(&stream);stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE);
	ComPtr<IWICBitmapEncoder> encoder;factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
	ComPtr<IWICBitmapFrameEncode> frame;ComPtr<IPropertyBag2> props;encoder->CreateNewFrame(&frame, &props);frame->Initialize(props.Get());
	frame->SetSize(w, h);
	WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;frame->SetPixelFormat(&fmt);
	frame->WritePixels(h, w * 4, (UINT)((size_t)w * h * 4), (BYTE*)rgbaBGRA.data());
	frame->Commit();encoder->Commit();
	return true;
}
#if SR_HAS_ONNX
class UnifiedAiEngine {
public:
	bool Init(const AiModelDef& def, ID3D11Device* dev, ID3D11DeviceContext* ctx) {
		StopThread();m_ready = false;m_dmlDead = false;m_def = def;m_dev = dev;m_ctx = ctx;
		try {
			if (!CreateSessions(true)) { m_dmlDead = true;if (!CreateSessions(false))return false; }
			Ort::AllocatorWithDefaultOptions alloc;auto inA = m_sess.GetInputNameAllocated(0, alloc);auto outA = m_sess.GetOutputNameAllocated(0, alloc);
			if (inA)m_inName = inA.get();if (outA)m_outName = outA.get();m_ready = true;
		}
		catch (const std::exception& e) { m_lastErr = e.what();return false; }
		catch (...) { m_lastErr = "Unknown init error";return false; }
		StartThread();return true;
	}
	~UnifiedAiEngine() { StopThread(); }
	void SetStrength(float s) { m_strength.store(std::clamp(s, 0.05f, 1.0f)); }
	void SetInterval(int ms) { m_interval.store(std::max(120, ms)); }
	void SetQuality(int q) { static const int kIn[4] = { 128,192,256,320 };m_maxIn.store(kIn[std::clamp(q, 0, 3)]); }
	bool Submit(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11Texture2D* frame) {
		if (!m_ready || !dev || !ctx || !frame)return false;auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastSub).count() < m_interval.load())return false;
		{ std::unique_lock<std::mutex> lk(m_inMtx, std::try_to_lock);if (!lk.owns_lock() || m_hasIn)return false; }
		D3D11_TEXTURE2D_DESC sd{};frame->GetDesc(&sd);
		if (!m_staging || sd.Width != m_stW || sd.Height != m_stH) {
			m_staging.Reset();D3D11_TEXTURE2D_DESC st = sd;st.Usage = D3D11_USAGE_STAGING;st.BindFlags = 0;st.CPUAccessFlags = D3D11_CPU_ACCESS_READ;st.MiscFlags = 0;
			if (FAILED(dev->CreateTexture2D(&st, nullptr, &m_staging)))return false;m_stW = sd.Width;m_stH = sd.Height;
		}
		ctx->CopyResource(m_staging.Get(), frame); { std::lock_guard<std::mutex> lk(m_inMtx);m_hasIn = true; }m_lastSub = now;m_cv.notify_one();return true;
	}
	bool TryGetResult(std::vector<uint8_t>& out, int& w, int& h, double& ms) { std::lock_guard<std::mutex> lk(m_outMtx);if (!m_hasOut)return false;out = m_out;w = m_outW;h = m_outH;ms = m_lastMs;m_hasOut = false;return true; }
	bool IsReady()const { return m_ready; }
	bool HasOutput()const { std::lock_guard<std::mutex> lk(m_outMtx);return m_hasOut; }
	std::string LastErr()const { return m_lastErr; }
	bool UsingCpu()const { return m_dmlDead; }
private:
	Ort::SessionOptions MakeOpts(bool dml) {
		Ort::SessionOptions opts;opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);opts.SetIntraOpNumThreads(1);opts.DisableMemPattern();opts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
		if (dml) { try { OrtStatus* st = OrtSessionOptionsAppendExecutionProvider_DML(opts, 0);if (st)Ort::GetApi().ReleaseStatus(st); } catch (...) {} }return opts;
	}
	bool CreateSessions(bool dml) { try { Ort::SessionOptions opts = MakeOpts(dml);m_sess = Ort::Session(GetOrtEnv(), m_def.localPath, opts);return true; } catch (...) { return false; } }
	void StartThread() { m_stop = false;m_hasIn = false;m_hasOut = false;m_thread = std::thread([this] {Loop();}); }
	void StopThread() { if (!m_thread.joinable())return;m_stop.store(true);m_cv.notify_one();m_thread.join(); }
	void Loop() {
		for (;;) {
			std::vector<uint8_t> rgba;int w = 0, h = 0;
			{
				std::unique_lock<std::mutex> lk(m_inMtx);m_cv.wait(lk, [this] {return m_hasIn || m_stop.load();});if (m_stop.load())break;
				D3D11_MAPPED_SUBRESOURCE mp{};
				{
					std::lock_guard<std::mutex> ctxLk(g_ctxMtx);if (m_ctx && m_staging && SUCCEEDED(m_ctx->Map(m_staging.Get(), 0, D3D11_MAP_READ, 0, &mp)) && mp.pData) {
						w = static_cast<int>(m_stW);h = static_cast<int>(m_stH);rgba.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
						for (int y = 0;y < h;++y)memcpy(&rgba[static_cast<size_t>(y) * static_cast<size_t>(w) * 4], static_cast<const uint8_t*>(mp.pData) + static_cast<size_t>(y) * mp.RowPitch, static_cast<size_t>(w) * 4);
						m_ctx->Unmap(m_staging.Get(), 0);
					}
				}
				m_hasIn = false;
			}
			if (rgba.empty())continue;auto t0 = std::chrono::steady_clock::now();
			try {
				std::vector<uint8_t> outImg;int ow = 0, oh = 0;RunSuperRes(rgba, w, h, outImg, ow, oh);double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
				std::lock_guard<std::mutex> lk(m_outMtx);m_out = std::move(outImg);m_outW = ow;m_outH = oh;m_lastMs = ms;m_hasOut = true;
			}
			catch (const std::exception& e) { if (!m_dmlDead) { m_dmlDead = true;try { if (CreateSessions(false))continue; } catch (...) {} }m_lastErr = e.what();m_ready = false; }
			catch (...) { m_lastErr = "AI runtime error";m_ready = false; }
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	static void BgraToNchw(const std::vector<uint8_t>& rgba, int w, int h, std::vector<float>& out, int dW, int dH) {
		size_t plane = static_cast<size_t>(dW) * static_cast<size_t>(dH);
		for (int y = 0;y < dH;++y) {
			int sy = std::min(h - 1, static_cast<int>((static_cast<int64_t>(y) * h) / dH));for (int x = 0;x < dW;++x) {
				int sx = std::min(w - 1, static_cast<int>((static_cast<int64_t>(x) * w) / dW));
				const uint8_t* px = &rgba[(static_cast<size_t>(sy) * w + sx) * 4];size_t idx = static_cast<size_t>(y) * dW + x;out[0 * plane + idx] = px[2] / 255.0f;out[1 * plane + idx] = px[1] / 255.0f;out[2 * plane + idx] = px[0] / 255.0f;
			}
		}
	}
	static void NchwToBgra(const float* nchw, int w, int h, std::vector<uint8_t>& out) {
		size_t plane = static_cast<size_t>(w) * static_cast<size_t>(h);
		for (int y = 0;y < h;++y)for (int x = 0;x < w;++x) {
			size_t idx = static_cast<size_t>(y) * w + x;float r = std::clamp(nchw[0 * plane + idx], 0.0f, 1.0f);float g = std::clamp(nchw[1 * plane + idx], 0.0f, 1.0f);float b = std::clamp(nchw[2 * plane + idx], 0.0f, 1.0f);
			uint8_t* o = &out[idx * 4];o[0] = static_cast<uint8_t>(b * 255.0f);o[1] = static_cast<uint8_t>(g * 255.0f);o[2] = static_cast<uint8_t>(r * 255.0f);o[3] = 255;
		}
	}
	void RunSuperRes(const std::vector<uint8_t>& rgba, int w, int h, std::vector<uint8_t>& out, int& ow, int& oh) {
		const int SCALE = std::max(1, m_def.outputScale);int maxIn = m_maxIn.load();int inW = std::clamp(w / SCALE, 64, maxIn);int inH = std::clamp(static_cast<int>((static_cast<int64_t>(inW) * h) / std::max(1, w)), 64, maxIn);
		inW = (inW / 8) * 8;inH = (inH / 8) * 8;std::vector<float> input(3LL * inW * inH, 0.0f);BgraToNchw(rgba, w, h, input, inW, inH);
		Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);std::array<int64_t, 4> shape{ 1,3,inH,inW };
		Ort::Value inT = Ort::Value::CreateTensor<float>(mi, input.data(), input.size(), shape.data(), shape.size());const char* inN[] = { m_inName.c_str() };const char* outN[] = { m_outName.c_str() };
		auto outs = m_sess.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN, 1);float* raw = outs[0].GetTensorMutableData<float>();ow = inW * SCALE;oh = inH * SCALE;out.resize(static_cast<size_t>(ow) * static_cast<size_t>(oh) * 4);NchwToBgra(raw, ow, oh, out);
	}
	Ort::Session m_sess{ nullptr };std::string m_inName, m_outName, m_lastErr;AiModelDef m_def{};bool m_ready = false;bool m_dmlDead = false;ID3D11Device* m_dev = nullptr;ID3D11DeviceContext* m_ctx = nullptr;
	ComPtr<ID3D11Texture2D> m_staging;UINT m_stW = 0, m_stH = 0;std::thread m_thread;std::mutex m_inMtx;std::condition_variable m_cv;bool m_hasIn = false;std::atomic<bool> m_stop{ false };
	mutable std::mutex m_outMtx;std::vector<uint8_t> m_out;int m_outW = 0, m_outH = 0;double m_lastMs = 0.0;bool m_hasOut = false;
	std::atomic<float> m_strength{ 0.60f };std::atomic<int> m_interval{ 500 };std::atomic<int> m_maxIn{ 192 };std::chrono::steady_clock::time_point m_lastSub{};
};
class DepthEngine {
public:
	bool Init(int desiredH, const std::wstring& modelPath, ID3D11DeviceContext* ctx) {
		StopThread();m_ready = false;m_ctx = ctx;auto snap = [](int v) {return std::max(14, (v / 14) * 14);};m_sz = snap(desiredH);
		try { if (!CreateSession(modelPath, false))return false;Ort::AllocatorWithDefaultOptions alloc;auto inA = m_sess.GetInputNameAllocated(0, alloc);auto outA = m_sess.GetOutputNameAllocated(0, alloc);if (inA)m_inName = inA.get();if (outA)m_outName = outA.get();m_ready = true; }
		catch (const std::exception& e) { m_lastErr = e.what();return false; }StartThread();return true;
	}
	~DepthEngine() { StopThread(); }
	bool Submit(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11Texture2D* frame) {
		if (!m_ready || !frame)return false; { std::unique_lock<std::mutex> lk(m_inMtx, std::try_to_lock);if (!lk.owns_lock() || m_hasIn)return false; }
		D3D11_TEXTURE2D_DESC sd{};frame->GetDesc(&sd);
		if (!m_staging || sd.Width != m_stagW || sd.Height != m_stagH) {
			m_staging.Reset();D3D11_TEXTURE2D_DESC st = sd;st.Usage = D3D11_USAGE_STAGING;st.BindFlags = 0;st.CPUAccessFlags = D3D11_CPU_ACCESS_READ;st.MiscFlags = 0;
			if (FAILED(dev->CreateTexture2D(&st, nullptr, &m_staging)))return false;m_stagW = sd.Width;m_stagH = sd.Height;
		}
		ctx->CopyResource(m_staging.Get(), frame); { std::lock_guard<std::mutex> lk(m_inMtx);m_hasIn = true; }m_cv.notify_one();return true;
	}
	bool TryGetResult(std::vector<float>& depth, double& ms) { std::lock_guard<std::mutex> lk(m_outMtx);if (!m_hasOut)return false;depth = m_depth;ms = m_ms;m_hasOut = false;return true; }
	bool IsReady()const { return m_ready; }int Size()const { return m_sz; }std::string LastErr()const { return m_lastErr; }
private:
	bool CreateSession(const std::wstring& p, bool dml) {
		try {
			Ort::SessionOptions opts;opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);opts.SetIntraOpNumThreads(1);opts.DisableMemPattern();opts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
			if (dml) { try { OrtStatus* st = OrtSessionOptionsAppendExecutionProvider_DML(opts, 0);if (st)Ort::GetApi().ReleaseStatus(st); } catch (...) {} }m_sess = Ort::Session(GetOrtEnv(), p.c_str(), opts);return true;
		}
		catch (...) { return false; }
	}
	void StartThread() { m_stop = false;m_hasIn = false;m_hasOut = false;m_thread = std::thread([this] {InfLoop();}); }
	void StopThread() { if (!m_thread.joinable())return;m_stop.store(true);m_cv.notify_one();m_thread.join(); }
	void Preprocess(const std::vector<uint8_t>& bgra, int sW, int sH, int dSz, std::vector<float>& out) {
		static const float mean[3] = { 0.485f,0.456f,0.406f };static const float stdv[3] = { 0.229f,0.224f,0.225f };size_t plane = static_cast<size_t>(dSz) * static_cast<size_t>(dSz);
		for (int y = 0;y < dSz;++y) {
			int sy = std::min(sH - 1, static_cast<int>((static_cast<int64_t>(y) * sH) / dSz));for (int x = 0;x < dSz;++x) {
				int sx = std::min(sW - 1, static_cast<int>((static_cast<int64_t>(x) * sW) / dSz));
				const uint8_t* px = &bgra[(static_cast<size_t>(sy) * sW + sx) * 4];float r = px[2] / 255.0f;float g = px[1] / 255.0f;float b = px[0] / 255.0f;size_t idx = static_cast<size_t>(y) * dSz + x;
				out[0 * plane + idx] = (r - mean[0]) / stdv[0];out[1 * plane + idx] = (g - mean[1]) / stdv[1];out[2 * plane + idx] = (b - mean[2]) / stdv[2];
			}
		}
	}
	void InfLoop() {
		for (;;) {
			std::vector<uint8_t> rgba;int w = 0, h = 0;
			{
				std::unique_lock<std::mutex> lk(m_inMtx);m_cv.wait(lk, [this] {return m_hasIn || m_stop.load();});if (m_stop.load())break;
				D3D11_MAPPED_SUBRESOURCE mp{};
				{
					std::lock_guard<std::mutex> ctxLk(g_ctxMtx);if (m_ctx && m_staging && SUCCEEDED(m_ctx->Map(m_staging.Get(), 0, D3D11_MAP_READ, 0, &mp)) && mp.pData) {
						w = static_cast<int>(m_stagW);h = static_cast<int>(m_stagH);rgba.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
						for (int y = 0;y < h;++y)memcpy(&rgba[static_cast<size_t>(y) * static_cast<size_t>(w) * 4], static_cast<const uint8_t*>(mp.pData) + static_cast<size_t>(y) * mp.RowPitch, static_cast<size_t>(w) * 4);m_ctx->Unmap(m_staging.Get(), 0);
					}
				}
				m_hasIn = false;
			}
			if (rgba.empty())continue;auto t0 = std::chrono::steady_clock::now();std::vector<float> input(3LL * m_sz * m_sz);Preprocess(rgba, w, h, m_sz, input);
			try {
				std::array<int64_t, 4> shape{ 1,3,m_sz,m_sz };Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
				Ort::Value inT = Ort::Value::CreateTensor<float>(mi, input.data(), input.size(), shape.data(), shape.size());const char* inN[] = { m_inName.c_str() };const char* outN[] = { m_outName.c_str() };
				auto outs = m_sess.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN, 1);float* raw = outs[0].GetTensorMutableData<float>();size_t cnt = static_cast<size_t>(m_sz) * static_cast<size_t>(m_sz);
				float mn = *std::min_element(raw, raw + cnt);float mx = *std::max_element(raw, raw + cnt);float rng = std::max(1e-6f, mx - mn);std::vector<float> depth(cnt);if (m_ema.size() != cnt)m_ema.assign(cnt, 0.5f);
				for (size_t i = 0;i < cnt;++i) { float t = (raw[i] - mn) / rng;m_ema[i] = m_ema[i] * 0.70f + t * 0.30f;depth[i] = m_ema[i]; }
				double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();std::lock_guard<std::mutex> lk(m_outMtx);m_depth = std::move(depth);m_ms = ms;m_hasOut = true;
			}
			catch (...) {}std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	Ort::Session m_sess{ nullptr };std::string m_inName, m_outName, m_lastErr;int m_sz = 384;bool m_ready = false;ID3D11DeviceContext* m_ctx = nullptr;ComPtr<ID3D11Texture2D> m_staging;UINT m_stagW = 0, m_stagH = 0;
	std::thread m_thread;std::mutex m_inMtx;std::condition_variable m_cv;bool m_hasIn = false;std::atomic<bool> m_stop{ false };std::mutex m_outMtx;std::vector<float> m_depth, m_ema;double m_ms = 0.0;bool m_hasOut = false;
};
#else
class UnifiedAiEngine {
public:
	bool Init(const AiModelDef&, ID3D11Device*, ID3D11DeviceContext*) { m_lastErr = "ONNX Runtime / DirectML headers not found";return false; }
	~UnifiedAiEngine() = default;
	void SetStrength(float) {}void SetInterval(int) {}void SetQuality(int) {}
	bool Submit(ID3D11Device*, ID3D11DeviceContext*, ID3D11Texture2D*) { return false; }
	bool TryGetResult(std::vector<uint8_t>&, int&, int&, double&) { return false; }
	bool IsReady()const { return false; }
	bool HasOutput()const { return false; }
	std::string LastErr()const { return m_lastErr; }
	bool UsingCpu()const { return false; }
private: std::string m_lastErr;
};
class DepthEngine {
public:
	bool Init(int, const std::wstring&, ID3D11DeviceContext*) { m_lastErr = "ONNX Runtime / DirectML headers not found";return false; }
	~DepthEngine() = default;
	bool Submit(ID3D11Device*, ID3D11DeviceContext*, ID3D11Texture2D*) { return false; }
	bool TryGetResult(std::vector<float>&, double&) { return false; }
	bool IsReady()const { return false; }
	int Size()const { return 128; }
	std::string LastErr()const { return m_lastErr; }
private: std::string m_lastErr;
};
#endif
struct alignas(16)EffectsCB {
	float glossyIntensity, glossyRoughness, glossyEnabled, vibranceAmount;
	float horizonY, skyGlowStrength, engineMode, themeMode;
	float fogEnabled, viewportWidth, viewportHeight, textProtectEnabled;
	float dofFocus, dofBlurStrength, dofEnabled, glossyFresnelPower;
	float glossySpecularGlint, glossyContrast, glossyTintR, glossyTintG;
	float glossyTintB, clearCoatMode, waveDistortionAmount, waveSpeed;
	float time, bloomIntensity, bloomThreshold, chromaticAberration;
	float vignetteIntensity, vignetteSmoothness, filmGrainAmount, sharpeningAmount;
	float exposure, fogDistance, contrastBoost, tempKelvin;
	float outlineEnabled, outlineThickness, outlineColorR, outlineColorG;
	float outlineColorB, splitScreenPos, splitScreenEnabled, specularTintR;
	float specularTintG, specularTintB, groundFadeEnd, tonemapMode;
	float ssaoIntensity, ssaoRadius, ssaoEnabled, dofRange;
	float fogDensity, fogColorR, fogColorG, fogColorB;
	float waveScale, neonGlowIntensity, neonPinkBoost, neonBlueFog;
	float upscaleFSR, upscaleDLSS, upscaleNAFNet, upscaleSD;
	float styleBlendStrength, upscaleTemporalSharpen, upscaleQuality, sharpeningRadius;
	float fxaaEnabled, godRayIntensity, godRayDecay, motionBlurAmount;
	float godRayX, godRayY, fsrSharpness, styleAspect;
	float scanlineIntensity, crtCurve, glitchAmount, zoomBlur;
	float tiltShift, panini, sunAngle, ambientLight;
	float rainDrops, windDistortion, snowAmount, nightVision;
	float thermalVision, shadowTintR, shadowTintG, shadowTintB;
	float matRoughness, matMetalness, bloomSpectrum, lensDirt;
	float customFx[40];
	float styleToon, styleRim, stylePalette, styleEdge;
	float styleShadowTintR, styleShadowTintG, styleShadowTintB, styleHighlightTintR;
	float styleHighlightTintG, styleHighlightTintB, styleWarmCool, styleNeon;
};
static const char* kVsSrc = R"(
struct VSOut{float4 pos:SV_Position;float2 uv:TEXCOORD0;};
VSOut main(uint id:SV_VertexID){
 VSOut o;
 o.uv = float2((id==1)?2.0:0.0,(id==2)?2.0:0.0);
 o.pos = float4(o.uv*float2(2,-2)+float2(-1,1),0,1);
 return o;
})";
static const char* kPsSrc = R"(
Texture2D SceneTex:register(t0);
Texture2D DepthTex:register(t1);
Texture2D StyleTex:register(t2);
Texture2D HistoryTex:register(t3);
Texture2D MaskTex:register(t4);
SamplerState LinSamp:register(s0);
cbuffer Effects:register(b0){
 float glossyIntensity,glossyRoughness,glossyEnabled,vibranceAmount;
 float horizonY,skyGlowStrength,engineMode,themeMode;
 float fogEnabled,viewportWidth,viewportHeight,textProtectEnabled;
 float dofFocus,dofBlurStrength,dofEnabled,glossyFresnelPower;
 float glossySpecularGlint,glossyContrast,glossyTintR,glossyTintG;
 float glossyTintB,clearCoatMode,waveDistortionAmount,waveSpeed;
 float time,bloomIntensity,bloomThreshold,chromaticAberration;
 float vignetteIntensity,vignetteSmoothness,filmGrainAmount,sharpeningAmount;
 float exposure,fogDistance,contrastBoost,tempKelvin;
 float outlineEnabled,outlineThickness,outlineColorR,outlineColorG;
 float outlineColorB,splitScreenPos,splitScreenEnabled,specularTintR;
 float specularTintG,specularTintB,groundFadeEnd,tonemapMode;
 float ssaoIntensity,ssaoRadius,ssaoEnabled,dofRange;
 float fogDensity,fogColorR,fogColorG,fogColorB;
 float waveScale,neonGlowIntensity,neonPinkBoost,neonBlueFog;
 float upscaleFSR,upscaleDLSS,upscaleNAFNet,upscaleSD;
 float styleBlendStrength,upscaleTemporalSharpen,upscaleQuality,sharpeningRadius;
 float fxaaEnabled,godRayIntensity,godRayDecay,motionBlurAmount;
 float godRayX,godRayY,fsrSharpness,styleAspect;
 float scanlineIntensity,crtCurve,glitchAmount,zoomBlur;
 float tiltShift,panini,sunAngle,ambientLight;
 float rainDrops,windDistortion,snowAmount,nightVision;
 float thermalVision,shadowTintR,shadowTintG,shadowTintB;
 float matRoughness,matMetalness,bloomSpectrum,lensDirt;
 float4 customFx[10];
 float styleToon,styleRim,stylePalette,styleEdge;
 float styleShadowTintR,styleShadowTintG,styleShadowTintB,styleHighlightTintR;
 float styleHighlightTintG,styleHighlightTintB,styleWarmCool,styleNeon;
};
struct PSIn{float4 pos:SV_Position;float2 uv:TEXCOORD0;};
float Lum(float3 c){return dot(c,float3(0.299,0.587,0.114));}
float Hash(float2 p){p=frac(p*float2(123.34,456.21));p+=dot(p,p+45.32);return frac(p.x*p.y);}
float3 RgbToHsv(float3 c){float4 K=float4(0.,-1./3.,2./3.,-1.);float4 p=lerp(float4(c.bg,K.wz),float4(c.gb,K.xy),step(c.b,c.g));float4 q=lerp(float4(p.xyw,c.r),float4(c.r,p.yzx),step(p.x,c.r));float d=q.x-min(q.w,q.y);float e=1e-10;return float3(abs(q.z+(q.w-q.y)/(6.*d+e)),d/(q.x+e),q.x);}
float3 HsvToRgb(float3 c){float4 K=float4(1.,2./3.,1./3.,3.);float3 p=abs(frac(c.xxx+K.xyz)*6.-K.www);return c.z*lerp(K.xxx,saturate(p-K.xxx),c.y);}
float3 ApplyTemp(float3 c,float k){float t=(k-6500)/3500.0;float3 f;if (t>0)f=float3(1+t*0.15,1,1-t*0.15);else f=float3(1-t*0.20,1+t*0.05,1+t*0.25);return c*f;}
float3 ApplyTonemap(float3 c,float m){if (m>0.5&&m<1.5){float a=2.51,b=0.03,cc=2.43,d=0.59,e=0.14;return saturate((c*(a*c+b))/(c*(cc*c+d)+e));}if (m>1.5&&m<2.5)return c/(c+1);if (m>2.5&&m<3.5){float3 x=max(0,c-0.004);return (x*(6.2*x+0.5))/(x*(6.2*x+1.7)+0.06);}return c;}
float3 EnhanceColors(float3 c,float v){float l=dot(c,float3(0.2126,0.7152,0.0722));return saturate(lerp(float3(l,l,l),c,1.40+v*0.50));}
float3 ApplyTheme(float3 c,float m){float l=dot(c,float3(0.2126,0.7152,0.0722));if (m<0.5)return c;if (m<1.5)return saturate(pow(c,float3(1.1,1.05,1.0))*1.15);if (m<2.5){float3 g=c*float3(1.30,0.95,0.65);return saturate(lerp(c,g,0.70)+float3(0.10,0.04,0)*l);}if (m<3.5){float3 cy=c*float3(0.70,1.15,1.35);cy=lerp(cy,cy*float3(1.30,0.60,1.20),saturate(l-0.3));return saturate(cy);}if (m<4.5)return saturate(c*float3(1.03,1.04,1.01)+float3(0.02,0.02,0.01));if (m<5.5)return saturate(c*float3(1.25,0.70,1.30));if (m<6.5)return saturate(pow(c,float3(1.2,1.1,1.3))*float3(0.85,1.1,1.4));if (m<7.5)return saturate(lerp(c,float3(l*0.4,l*1.35,l*0.5),0.75));if (m<8.5)return saturate(float3(l,l,l)*1.15);if (m<9.5)return saturate(lerp(float3(l,l,l),c,1.85)*1.08);if (m<10.5){float3 s=lerp(c,c*float3(0.6,1.1,1.25),saturate(1-l*2));return saturate(lerp(s,c*float3(1.3,0.9,0.6),saturate((l-0.4)*2)));}if (m<11.5)return saturate(c*float3(1.15,0.92,0.78)+float3(0.05,0.02,0.01));if (m<12.5)return saturate(lerp(c,c*float3(1.1,1.05,1.15)+float3(0.08,0.08,0.1),0.5));if (m<13.5)return saturate(c*float3(0.82,0.90,1.05));if (m<14.5)return saturate(float3(saturate(l*1.8),saturate(1-abs(l-0.5)*2.5),saturate((1-l)*1.5)));if (m<15.5)return saturate(lerp(c,float3(l*1.2,l*0.95,l*0.7),0.70));if (m<16.5)return saturate(c*float3(0.70,1.10,1.40)+float3(0,0.03,0.08));if (m<17.5)return saturate(pow(c,float3(1.35,1.35,1.35))*0.85);if (m<18.5)return saturate(c*float3(0.75,1.25,0.85));if (m<19.5)return saturate(c*float3(1.40,0.65,0.40));if (m<20.5)return saturate(c*float3(1.10,0.70,1.35));if (m<21.5){float3 h=pow(c,float3(1.15,1.15,1.15));return saturate(lerp(float3(l,l,l),h,1.6)*1.1);}return c;}
float3 FxaaPass(float2 uv,float2 tx){
 float3 rgbNW=SceneTex.SampleLevel(LinSamp,uv+float2(-1,-1)*tx,0).rgb;float3 rgbNE=SceneTex.SampleLevel(LinSamp,uv+float2(1,-1)*tx,0).rgb;
 float3 rgbSW=SceneTex.SampleLevel(LinSamp,uv+float2(-1,1)*tx,0).rgb;float3 rgbSE=SceneTex.SampleLevel(LinSamp,uv+float2(1,1)*tx,0).rgb;float3 rgbM=SceneTex.SampleLevel(LinSamp,uv,0).rgb;
 float lNW=Lum(rgbNW),lNE=Lum(rgbNE),lSW=Lum(rgbSW),lSE=Lum(rgbSE),lM=Lum(rgbM);
 float lMin=min(lM,min(min(lNW,lNE),min(lSW,lSE)));float lMax=max(lM,max(max(lNW,lNE),max(lSW,lSE)));
 float2 dir=float2(-((lNW+lNE)-(lSW+lSE)),((lNW+lSW)-(lNE+lSE)));float dirReduce=max((lNW+lNE+lSW+lSE)*0.03125,0.0078125);float rcpMin=1.0/(min(abs(dir.x),abs(dir.y))+dirReduce);
 dir=min(float2(8,8),max(float2(-8,-8),dir*rcpMin))*tx;
 float3 rgbA=0.5*(SceneTex.SampleLevel(LinSamp,uv+dir*(1.0/3.0-0.5),0).rgb+SceneTex.SampleLevel(LinSamp,uv+dir*(2.0/3.0-0.5),0).rgb);
 float3 rgbB=rgbA*0.5+0.25*(SceneTex.SampleLevel(LinSamp,uv+dir*(-0.5),0).rgb+SceneTex.SampleLevel(LinSamp,uv+dir*0.5,0).rgb);
 float lB=Lum(rgbB);if (lB<lMin||lB>lMax)return rgbA;return rgbB;}
float3 CasPass(float2 uv,float2 tx,float sharp){
 float3 b=SceneTex.SampleLevel(LinSamp,uv+float2(0,-1)*tx,0).rgb;float3 d=SceneTex.SampleLevel(LinSamp,uv+float2(-1,0)*tx,0).rgb;float3 e=SceneTex.SampleLevel(LinSamp,uv,0).rgb;float3 f=SceneTex.SampleLevel(LinSamp,uv+float2(1,0)*tx,0).rgb;float3 h=SceneTex.SampleLevel(LinSamp,uv+float2(0,1)*tx,0).rgb;
 float mnL=min(Lum(b),min(Lum(d),min(Lum(e),min(Lum(f),Lum(h)))));float mxL=max(Lum(b),max(Lum(d),max(Lum(e),max(Lum(f),Lum(h)))));float amp=sqrt(saturate(min(mnL,1.0-mxL)/max(mxL,0.0001)));float w=-0.22*sharp*amp;return saturate((e+(b+d+f+h)*w)/(1.0+4.0*w));}
float CF(int i){return customFx[i/4][i%4];}
float4 main(PSIn i):SV_Target{
 float2 tx=float2(1/max(1,viewportWidth),1/max(1,viewportHeight));float2 uv=i.uv;
 if (panini>0.001){float2 p=uv*2.0-1.0;float squeeze=1.0+abs(p.x)*(panini*0.45);p.x/=squeeze;uv=p*0.5+0.5;}
 uv=clamp(uv,0.001,0.999);
 if (splitScreenEnabled>0.5 && upscaleSD<0.5){if (uv.x>splitScreenPos){if (abs(uv.x-splitScreenPos)<(2/max(1,viewportWidth)))return float4(1,1,1,1);return float4(SceneTex.SampleLevel(LinSamp,uv,0).rgb,1);}}
 if (crtCurve>0.001){float2 cc=uv-0.5;cc*=1.0+dot(cc,cc)*crtCurve*0.8;uv=clamp(cc+0.5,0.001,0.999);}
 if (glitchAmount>0.001){float g=step(0.95,sin(time*10.0+uv.y*100.0));uv.x+=g*glitchAmount*0.1;}
 float rainMask=0.0;float rainShade=0.0;
 if (rainDrops>0.001){
 float2 baseUV = uv;
 float wind = windDistortion*0.8;
 float t = time*(18.0 + rainDrops*28.0);
 float density = rainDrops*1.8;
 float2 uvWind = float2(uv.x + uv.y*wind*0.15,uv.y);
 float accum=0;
 for (int l=0;l<4;l++){
 float scale = 70.0 + float(l)*28.0;
 float speed = 0.85 + float(l)*0.30;
 float thin = 14.0 - float(l)*2.0;
 float2 ruv = float2(uvWind.x*viewportWidth*0.025*(scale/70.0)+ float(l)*2.7 + uvWind.y*wind*0.5,uvWind.y*viewportHeight*0.018*(scale/70.0)- t*speed + float(l)*3.3);
 float id = Hash(floor(ruv));
 float2 f = frac(ruv);
 float x = abs(f.x-0.5);
 float rainLine = 1.0 - saturate(x*thin);
 float y = f.y;
 float tail = exp(-y*1.5)*(1.0 - exp(-y*8.0));
 float spark = smoothstep(0.94,1.0,id);
 float drop = spark *rainLine* tail;
 float len = 0.7 + id*0.6;
 drop *= len;
 accum += drop *density*(0.5 + float(l)*0.18);
}
 float2 dropUV = float2(uv.x*viewportWidth*0.12,uv.y*viewportHeight*0.04 - t*0.4);
 float dId = Hash(floor(dropUV));
 float d = smoothstep(0.96,1.0,dId)*step(frac(dropUV.y),0.15)* 0.25 * density;
 accum += d;
 rainMask = saturate(accum);
 uv.x += rainMask*0.0035*(1.0+windDistortion*0.6);
 uv.y += rainMask*0.0008;
 rainShade = rainMask;
}
 if (windDistortion>0.001){uv.x+=sin(uv.y*20.0+time*2.0)*windDistortion*0.01;}
 float3 baseScene;
 if (chromaticAberration>0.001){float2 ca=(uv-0.5)*chromaticAberration*0.014;float r=SceneTex.SampleLevel(LinSamp,clamp(uv+ca,0.001,0.999),0).r;float g=SceneTex.SampleLevel(LinSamp,uv,0).g;float b=SceneTex.SampleLevel(LinSamp,clamp(uv-ca,0.001,0.999),0).b;baseScene=float3(r,g,b);}else {baseScene=SceneTex.SampleLevel(LinSamp,uv,0).rgb;}
 if (fxaaEnabled>0.5)baseScene=FxaaPass(uv,tx);
 if (zoomBlur>0.001){float2 dir=(uv-0.5)*zoomBlur*0.035;float3 zb=baseScene;[unroll]for (int z=1;z<=4;z++){float tt=z/4.0;zb+=SceneTex.SampleLevel(LinSamp,clamp(uv-dir*tt,0.001,0.999),0).rgb;}baseScene=lerp(baseScene,zb/5.0,saturate(zoomBlur));}
 if (upscaleDLSS>0.5){float3 hist=HistoryTex.SampleLevel(LinSamp,uv,0).rgb;float3 mn=baseScene,mx=baseScene;[unroll]for (int yy=-1;yy<=1;yy++){[unroll]for (int xx=-1;xx<=1;xx++){float3 s=SceneTex.SampleLevel(LinSamp,uv+float2((float)xx,(float)yy)*tx,0).rgb;mn=min(mn,s);mx=max(mx,s);}}hist=clamp(hist,mn,mx);baseScene=lerp(baseScene,hist,0.55);if (upscaleTemporalSharpen>0.5){float3 blur=(mn+mx)*0.5;baseScene=saturate(baseScene+(baseScene-blur)*0.5);}}
 if (dofEnabled>0.5||tiltShift>0.001){float depth=DepthTex.SampleLevel(LinSamp,uv,0).r;float focus=tiltShift>0.001?tiltShift:dofFocus;float dd=max(0,abs(depth-focus)-dofRange);float blurAmt=saturate(dd/max(0.001,dofRange))*dofBlurStrength;if (blurAmt>0.003){float3 acc=float3(0,0,0);[unroll]for (int k=0;k<6;k++){float a=k*1.0472;float2 off=float2(cos(a),sin(a))*blurAmt*0.013;acc+=SceneTex.SampleLevel(LinSamp,clamp(uv+off,0.001,0.999),0).rgb;}baseScene=lerp(baseScene,acc/6.0,blurAmt);}}
 if (waveDistortionAmount>0.001){float wav=sin(uv.x*waveScale+time*waveSpeed*3.5)*cos(uv.y*(waveScale*0.6)+time*waveSpeed*2.5);float2 wOff=float2(wav*waveDistortionAmount*0.003,wav*waveDistortionAmount*0.0015);baseScene=SceneTex.SampleLevel(LinSamp,clamp(uv+wOff,0.001,0.999),0).rgb;}
 if (upscaleNAFNet>0.5){float3 sty=StyleTex.SampleLevel(LinSamp,uv,0).rgb;baseScene=lerp(baseScene,sty,saturate(styleBlendStrength));}
 if (upscaleSD>0.5){
 float sa = max(styleAspect,0.01);
 float va = viewportWidth/max(1.0,viewportHeight);
 float scale = max(va/sa,1.0);
 float2 aUV = clamp((uv - 0.5)/scale + 0.5,0.001,0.999);
 float3 aiC = StyleTex.SampleLevel(LinSamp,aUV,0).rgb;
 baseScene = lerp(SceneTex.SampleLevel(LinSamp,uv,0).rgb,aiC,saturate(styleBlendStrength));
}
 float2 res=float2(max(1,viewportWidth),max(1,viewportHeight));
 if (CF(0)>0.001){float ang=CF(1)*6.28318;float2 dir=float2(cos(ang),sin(ang))*tx*(2.0+CF(0)*16.0);float3 prism=float3(SceneTex.SampleLevel(LinSamp,clamp(uv+dir,0.001,0.999),0).r,baseScene.g,SceneTex.SampleLevel(LinSamp,clamp(uv-dir,0.001,0.999),0).b);baseScene=lerp(baseScene,prism,saturate(CF(0)));}
 if (CF(2)>0.001||CF(3)>0.001){float scale=lerp(8.0,42.0,saturate(CF(2)));float2 gv=frac(uv*res/scale)-0.5;float l=1.0-Lum(baseScene);float dotMask=smoothstep(l*0.45+0.02,l*0.45-0.10,length(gv));float3 ht=baseScene*dotMask+0.05;baseScene=lerp(baseScene,ht,saturate(CF(3)));}
 if (CF(4)>0.001||CF(5)>0.001){float block=lerp(2.0,28.0,saturate(CF(4)));float2 puv=(floor(uv*res/block)*block+block*0.5)/res;float3 pix=SceneTex.SampleLevel(LinSamp,clamp(puv,0.001,0.999),0).rgb;baseScene=lerp(baseScene,pix,saturate(CF(5)));}
 if (CF(6)>0.001||CF(7)>0.001){float lv=floor(2.0+saturate(CF(6))*10.0);float3 post=floor(baseScene*lv)/max(1.0,lv);baseScene=lerp(baseScene,post,saturate(CF(7)));}
 if (CF(8)>0.001||CF(9)>0.001){float2 eo=tx*(1.0+CF(8)*8.0);float3 e1=SceneTex.SampleLevel(LinSamp,clamp(uv+eo,0.001,0.999),0).rgb;float3 e2=SceneTex.SampleLevel(LinSamp,clamp(uv-eo,0.001,0.999),0).rgb;float3 emb=saturate((e1-e2)*2.0+0.5);baseScene=lerp(baseScene,emb,saturate(CF(9)));}
 if (CF(10)>0.001){float l=Lum(baseScene);float warm=CF(11);float3 a=lerp(float3(0.16,0.30,0.75),float3(0.95,0.52,0.18),warm);float3 b=lerp(float3(0.90,0.90,1.00),float3(1.00,0.92,0.75),warm);float3 duo=lerp(a,b,l);baseScene=lerp(baseScene,duo,saturate(CF(10)));}
 if (CF(12)>0.001){float ang=CF(13)*6.28318;float2 dir=float2(cos(ang),sin(ang))*tx*8.0;float3 streak=float3(0,0,0);float w=1.0;[unroll]for (int s=1;s<=5;s++){float tt=s/5.0;float3 smp=SceneTex.SampleLevel(LinSamp,clamp(uv+dir*tt*8.0,0.001,0.999),0).rgb;float hl=saturate(Lum(smp)-0.7);streak+=smp*hl*w;w*=0.72;}baseScene+=streak*CF(12)*0.25;}
 if (CF(14)>0.001||CF(15)>0.001){float scan=floor(uv.y*res.y*0.25);float jitter=(Hash(float2(scan,time*3.0))-0.5)*CF(14)*0.02;float3 vhs=float3(SceneTex.SampleLevel(LinSamp,clamp(uv+float2(jitter,0),0.001,0.999),0).r,SceneTex.SampleLevel(LinSamp,uv,0).g,SceneTex.SampleLevel(LinSamp,clamp(uv-float2(jitter,0),0.001,0.999),0).b);vhs+=((Hash(uv*res+time)-0.5)*0.08)*CF(15);baseScene=lerp(baseScene,vhs,saturate(max(CF(14),CF(15))));}
 if (CF(16)>0.001||CF(17)>0.001){float band=abs(uv.y-frac(time*0.20))*3.0/max(0.05,CF(16));float pulse=saturate(1.0-band)*CF(17);baseScene+=pulse*0.22;}
 if (CF(18)>0.001){float2 hOff=float2(sin(uv.y*40.0+time*(1.0+CF(19)*8.0)),cos(uv.x*30.0+time*(1.2+CF(19)*6.0)))*tx*(CF(18)*20.0);baseScene=lerp(baseScene,SceneTex.SampleLevel(LinSamp,clamp(uv+hOff,0.001,0.999),0).rgb,saturate(CF(18)));}
 if (CF(20)>0.001){float2 eo=tx*(1.0+CF(21)*12.0);float3 eA=SceneTex.SampleLevel(LinSamp,clamp(uv+float2(eo.x,0),0.001,0.999),0).rgb;float3 eB=SceneTex.SampleLevel(LinSamp,clamp(uv-float2(eo.x,0),0.001,0.999),0).rgb;float3 eC=SceneTex.SampleLevel(LinSamp,clamp(uv+float2(0,eo.y),0.001,0.999),0).rgb;float edge=length((eA-eB)+(eC-baseScene));baseScene+=float3(0.45,0.75,1.0)*saturate(edge*1.5)*CF(20);}
 if (CF(22)>0.001||CF(23)>0.001){float3 hsv=RgbToHsv(baseScene);float d=abs(hsv.x-CF(22));d=min(d,1.0-d);float keep=saturate(1.0-d/max(0.03,CF(23)*0.5));float3 muted=lerp(float3(Lum(baseScene),Lum(baseScene),Lum(baseScene)),baseScene,0.20);baseScene=lerp(muted,baseScene,keep);}
 if (CF(24)>0.001||CF(25)>0.001){float l=Lum(baseScene);float mask=saturate((l-CF(24))*3.0);float3 tint=float3(1.0,0.88,0.70);baseScene=lerp(baseScene,lerp(baseScene,tint,mask),CF(25));}
 if (CF(26)>0.001||CF(27)>0.001){float cell=lerp(6.0,40.0,saturate(CF(26)));float2 cuv=floor(uv*res/cell)*cell/res;float3 cry=SceneTex.SampleLevel(LinSamp,clamp(cuv,0.001,0.999),0).rgb;baseScene=lerp(baseScene,cry,saturate(CF(27)));}
 if (CF(28)>0.001){float r=length(uv-0.5)/max(0.1,lerp(1.2,0.25,CF(29)));float t=saturate(r)*CF(28);baseScene=lerp(baseScene,baseScene*float3(1.08,0.95,1.18)+float3(0.04,0.01,0.08),t);}
 if (CF(30)>0.001){float l=Lum(baseScene);float crush=saturate((CF(31)-l)*2.0)*CF(30);baseScene*=1.0-crush*0.7;}
 if (CF(32)>0.001){float3 blur=(SceneTex.SampleLevel(LinSamp,clamp(uv+tx*2.0,0.001,0.999),0).rgb+SceneTex.SampleLevel(LinSamp,clamp(uv-tx*2.0,0.001,0.999),0).rgb+SceneTex.SampleLevel(LinSamp,clamp(uv+float2(tx.x,-tx.y)*2.0,0.001,0.999),0).rgb+SceneTex.SampleLevel(LinSamp,clamp(uv+float2(-tx.x,tx.y)*2.0,0.001,0.999),0).rgb)*0.25;float hl=saturate((Lum(baseScene)-CF(33))*4.0);baseScene=lerp(baseScene,blur,hl*CF(32));}
 if (CF(34)>0.001){float rr=length(uv-0.5)/max(0.1,lerp(1.2,0.25,CF(29)));float tunnel=smoothstep(CF(34),CF(34)+max(0.05,CF(35)*0.6),rr);baseScene*=1.0-tunnel*0.65;}
 if (CF(36)>0.001){float2 p2=uv*2.0-1.0;float r2=dot(p2,p2);p2*=1.0+r2*(CF(36)*(0.4+CF(37)));float2 wuv=clamp(p2*0.5+0.5,0.001,0.999);baseScene=lerp(baseScene,SceneTex.SampleLevel(LinSamp,wuv,0).rgb,saturate(CF(36)));}
 if (vibranceAmount>0.001)baseScene=EnhanceColors(baseScene,vibranceAmount);baseScene=ApplyTheme(baseScene,themeMode);
 float3 colorAccum=baseScene;
 colorAccum+=ambientLight*0.035;
 float2 sunDir=float2(cos(radians(sunAngle)),sin(radians(sunAngle)));
 float2 sunDelta=uv-0.5;float sunLen=max(length(sunDelta),0.001);float2 sunVec=sunDelta/sunLen;
 float sunGlow=saturate(dot(sunVec,sunDir)*0.5+0.5);
 colorAccum+=float3(1.0,0.86,0.68)*sunGlow*skyGlowStrength*0.08;
 if (ssaoEnabled>0.5||contrastBoost>0.001){float dc=DepthTex.SampleLevel(LinSamp,uv,0).r;float2 rad=tx*max(1.0,ssaoRadius*2.0);float occ=0.0;occ+=saturate((DepthTex.SampleLevel(LinSamp,clamp(uv+float2(rad.x,0),0.001,0.999),0).r-dc)*3.0);occ+=saturate((DepthTex.SampleLevel(LinSamp,clamp(uv-float2(rad.x,0),0.001,0.999),0).r-dc)*3.0);occ+=saturate((DepthTex.SampleLevel(LinSamp,clamp(uv+float2(0,rad.y),0.001,0.999),0).r-dc)*3.0);occ+=saturate((DepthTex.SampleLevel(LinSamp,clamp(uv-float2(0,rad.y),0.001,0.999),0).r-dc)*3.0);occ=saturate(occ*0.25*ssaoIntensity);float sh=saturate(occ+occ*contrastBoost);float3 shTint=lerp(float3(1,1,1),float3(shadowTintR,shadowTintG,shadowTintB),0.35);colorAccum*=lerp(1.0,1.0-sh*0.45,ssaoEnabled);colorAccum*=lerp(float3(1,1,1),shTint,sh*0.25);}
 if (glossyEnabled>0.5){float gW=saturate((uv.y-horizonY)*18.0);if (gW>0.0001){float normD=saturate((uv.y-horizonY)/max(0.0001,1-horizonY));gW*=1.0-smoothstep(groundFadeEnd*0.55,groundFadeEnd*0.55+0.35,normD);float rOff=max(0.001,uv.y-horizonY)*(0.38+glossyRoughness*0.15);float2 sUV=float2(uv.x,saturate(uv.y-rOff));float blurMip=(glossyRoughness+matRoughness*0.01)*normD*3.5;float3 sampC=SceneTex.SampleLevel(LinSamp,clamp(sUV,0.001,0.999),blurMip).rgb;if (textProtectEnabled>0.5){float mx=max(sampC.r,max(sampC.g,sampC.b));float mn=min(sampC.r,min(sampC.g,sampC.b));if ((mx-mn)<0.03&&(mx>0.94||mx<0.06)){float3 skyC=SceneTex.SampleLevel(LinSamp,float2(sUV.x,horizonY*0.5),2.0).rgb*float3(0.9,1.0,1.12);sampC=lerp(sampC,skyC,1.0);}}if (neonGlowIntensity>0.01){float3 nC=sampC*float3(1.4*neonPinkBoost,0.8,1.5);sampC=lerp(sampC,nC,saturate(neonGlowIntensity*0.65));}sampC=pow(max(sampC,0),float3(glossyContrast,glossyContrast,glossyContrast));sampC=lerp(sampC,sampC*float3(1.10,1.06,1.02),saturate(matMetalness*0.5));sampC*=float3(glossyTintR,glossyTintG,glossyTintB);if (clearCoatMode>0.5){sampC*=1.18;sampC+=float3(0.06,0.07,0.09)*glossySpecularGlint*0.6;}float fresnel=saturate(0.08+0.92*pow(normD,glossyFresnelPower));float mA=fresnel*gW*glossyIntensity*0.95;colorAccum=lerp(colorAccum,sampC,saturate(mA*0.75));}}
 if (skyGlowStrength>0.001){float dH=abs(uv.y-horizonY);float sM=saturate(1-dH*18.0);float3 sB=SceneTex.SampleLevel(LinSamp,float2(uv.x,horizonY),1.5).rgb;colorAccum+=sB*float3(1.15,1.05,0.85)*skyGlowStrength*sM*0.45;}
 if (outlineEnabled>0.5){float dC=DepthTex.SampleLevel(LinSamp,uv,0).r;float dR=DepthTex.SampleLevel(LinSamp,uv+float2(tx.x*outlineThickness,0),0).r;float dD=DepthTex.SampleLevel(LinSamp,uv+float2(0,tx.y*outlineThickness),0).r;float edge=(abs(dC-dR)+abs(dC-dD))*12;if (edge>0.12){float3 oC=float3(outlineColorR,outlineColorG,outlineColorB);float oa=saturate(edge*2.5)*0.9;colorAccum=lerp(colorAccum,oC,oa);}}
 if (godRayIntensity>0.005){float2 lpos=float2(godRayX,godRayY);float2 delta=(uv-lpos)*0.045;float2 suv=uv;float wgt=1.0;float3 acc=float3(0,0,0);[loop]for (int k=0;k<12;k++){suv-=delta;float3 s=SceneTex.SampleLevel(LinSamp,clamp(suv,0.001,0.999),2.0).rgb;float l=saturate(Lum(s)-bloomThreshold);acc+=s*l*wgt;wgt*=godRayDecay;}colorAccum+=acc*0.08*godRayIntensity*float3(1.0,0.96,0.88);}
 if (bloomIntensity>0.01){float3 bS=(SceneTex.SampleLevel(LinSamp,uv,3.0).rgb+SceneTex.SampleLevel(LinSamp,uv,4.0).rgb)*0.5;float bl=dot(bS,float3(0.2126,0.7152,0.0722));if (bl>bloomThreshold){float3 bTint=lerp(float3(1.0,1.0,1.0),float3(1.25,0.92,1.55),bloomSpectrum);float dirt=lerp(1.0,0.55+0.45*Hash(uv*96.0+time*0.03),lensDirt);colorAccum+=(bS-bloomThreshold)*bloomIntensity*1.5*bTint*dirt;}}
 if (neonBlueFog>0.005){float dd=DepthTex.SampleLevel(LinSamp,uv,0).r;float f=saturate(dd*1.4)*neonBlueFog;colorAccum=lerp(colorAccum,float3(0.12,0.30,0.95),f*0.45);}
 if (rainShade>0.001){colorAccum=lerp(colorAccum,colorAccum+float3(0.18,0.20,0.24),rainShade*0.65);}
 if (vignetteIntensity>0.01){float2 d=(uv-0.5)*1.3;float v=dot(d,d);float vf=smoothstep(0.15,0.75+(1-vignetteSmoothness)*0.5,v)*vignetteIntensity;colorAccum*=(1-vf);}
 if (filmGrainAmount>0.001){float noise=(Hash(uv+float2(time*0.11,time*0.07))-0.5)*filmGrainAmount*0.22;colorAccum+=float3(noise,noise,noise);}
 if (scanlineIntensity>0.001){colorAccum*=1.0-scanlineIntensity*abs(sin(uv.y*viewportHeight*0.5));}
 if (snowAmount>0.001){float2 suv=uv+float2(time*0.02,time*0.15);float sf=Hash(floor(suv*float2(180.0,100.0)));float snow=smoothstep(0.988,1.0,sf)*snowAmount;colorAccum=lerp(colorAccum,float3(1.0,1.0,1.0),snow);}
 if (nightVision>0.001){float l=Lum(colorAccum);colorAccum=lerp(colorAccum,float3(l*0.1,l*1.5,l*0.1),nightVision);}
 if (thermalVision>0.001){float l=Lum(colorAccum);colorAccum=lerp(colorAccum,float3(saturate(l*2),saturate(1-abs(l-0.5)*2.5),saturate((1-l)*1.5)),thermalVision);}
 if (motionBlurAmount>0.01){float3 hist=HistoryTex.SampleLevel(LinSamp,uv,0).rgb;colorAccum=lerp(colorAccum,hist,saturate(motionBlurAmount)*0.85);}
 colorAccum=ApplyTonemap(colorAccum,tonemapMode);if (abs(exposure)>0.01)colorAccum*=pow(2,exposure);if (abs(tempKelvin-6500)>20)colorAccum=ApplyTemp(colorAccum,tempKelvin);
 if (sharpeningAmount>0.001){float2 stx=tx*sharpeningRadius;float3 u=SceneTex.SampleLevel(LinSamp,uv-float2(0,stx.y),0).rgb;float3 d2=SceneTex.SampleLevel(LinSamp,uv+float2(0,stx.y),0).rgb;float3 l2=SceneTex.SampleLevel(LinSamp,uv-float2(stx.x,0),0).rgb;float3 r2=SceneTex.SampleLevel(LinSamp,uv+float2(stx.x,0),0).rgb;colorAccum=saturate(colorAccum+(colorAccum-(u+d2+l2+r2)*0.25)*sharpeningAmount*0.65);}
 if (upscaleFSR>0.5){float3 cas=CasPass(uv,tx,fsrSharpness);colorAccum=saturate(colorAccum+(cas-baseScene));}
 if (fogEnabled>0.5){float depth=DepthTex.SampleLevel(LinSamp,uv,0).r;float fogAmt=saturate((depth-fogDistance)/max(0.001,fogDensity));float3 fogC=float3(fogColorR,fogColorG,fogColorB);colorAccum=lerp(colorAccum,fogC,fogAmt*0.88);}
 if (CF(38)>0.001||CF(39)>0.001){float edge=min(min(uv.x,1.0-uv.x),min(uv.y,1.0-uv.y));float band=saturate((0.04+CF(38)*0.14-edge)*28.0);float pulse=0.55+0.45*sin(time*(1.0+CF(39)*8.0));colorAccum+=float3(1.0,0.55,0.08)*band*CF(38)*pulse*0.55;}
 float3 shT=float3(styleShadowTintR,styleShadowTintG,styleShadowTintB);
 float3 hiT=float3(styleHighlightTintR,styleHighlightTintG,styleHighlightTintB);
 if (styleToon>=2.0){
 float lv=floor(styleToon);
 float dith=Hash(uv*res+frac(time)*0.13)*0.06-0.03;
 float l=Lum(colorAccum)+dith;
 float q=floor(l*lv)/max(1.0,lv);
 colorAccum*=lerp(1.0,q/max(l,1e-4),0.9);
 float band=smoothstep(0.5,0.52,frac(l*lv));
 colorAccum*=1.0-band*0.12;
}
 if (stylePalette>0.01){
 float l=Lum(colorAccum);
 float3 duo=lerp(shT,hiT,smoothstep(0.0,1.0,l));
 colorAccum=lerp(colorAccum,duo,stylePalette);
 float2 hp=frac(uv*res/7.0)-0.5;
 float dotM=smoothstep(0.55,0.35,length(hp));
 colorAccum=lerp(colorAccum,colorAccum*0.85+hiT*0.5,stylePalette*(1.0-dotM)*0.25);
}
 if (styleRim>0.01){
 float dC=DepthTex.SampleLevel(LinSamp,uv,0).r;
 float dR=DepthTex.SampleLevel(LinSamp,uv+float2(tx.x*1.5,0),0).r;
 float dD=DepthTex.SampleLevel(LinSamp,uv+float2(0,tx.y*1.5),0).r;
 float edge=saturate((abs(dC-dR)+abs(dC-dD))*6.0);
 float lC=Lum(colorAccum);
 float lR=Lum(SceneTex.SampleLevel(LinSamp,uv+float2(tx.x,0),0).rgb);
 float lD=Lum(SceneTex.SampleLevel(LinSamp,uv+float2(0,tx.y),0).rgb);
 float lEdge=saturate((abs(lC-lR)+abs(lC-lD))*3.0);
 float lineM=saturate(edge*1.2+lEdge*1.5);
 colorAccum=lerp(colorAccum,colorAccum*0.25+shT*0.35,lineM*styleRim*0.55);
 float maskM=MaskTex.SampleLevel(LinSamp,uv,0).r;
 colorAccum=lerp(colorAccum,colorAccum+hiT*1.2,maskM*styleRim*0.45);
 colorAccum+=hiT*lineM*styleNeon*0.35;
}
 if (styleEdge>0.01){
 float3 bl=(SceneTex.SampleLevel(LinSamp,uv,2.0).rgb+SceneTex.SampleLevel(LinSamp,uv,3.0).rgb)*0.5;
 colorAccum=lerp(colorAccum,bl,styleEdge);
}
 if (styleRim>0.01){
 float dC=DepthTex.SampleLevel(LinSamp,uv,0).r;
 float spec=saturate((dC-0.5)*2.0)*styleRim;
 colorAccum=lerp(colorAccum,colorAccum+hiT*0.8,spec*0.18);
}
 if (styleWarmCool>0.02)colorAccum=lerp(colorAccum,colorAccum*float3(1.15,1.0,0.87),styleWarmCool);
 if (styleWarmCool<-0.02)colorAccum=lerp(colorAccum,colorAccum*float3(0.85,0.98,1.15),-styleWarmCool);
 if (stylePalette>0.3&&styleToon>=2.0)colorAccum*=1.0-0.12*abs(sin(uv.y*res.y*0.6));
 if (stylePalette>0.5)colorAccum+=(Hash(uv*res+time*0.11)-0.5)*0.03;
 return float4(saturate(colorAccum),1.0);
}
)";
static const char* kPsResizeSrc = R"(
Texture2D SceneTex:register(t0);
SamplerState LinSamp:register(s0);
struct PSIn{float4 pos:SV_Position;float2 uv:TEXCOORD0;};
float4 main(PSIn i):SV_Target{return float4(SceneTex.SampleLevel(LinSamp,i.uv,0).rgb,1.0);}
)";
struct RszSlot {
	ComPtr<ID3D11Texture2D> tex;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11ShaderResourceView> srv;
	UINT w = 0, h = 0;
};
class CompositingPipeline {
public:
	bool Init(ID3D11Device* dev) {
		m_lastErr.clear();if (!dev)return false;ComPtr<ID3DBlob> vsB, psB, errB;
		if (FAILED(D3DCompile(kVsSrc, strlen(kVsSrc), "VS", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vsB, &errB))) { if (errB && errB->GetBufferSize() && errB->GetBufferPointer())m_lastErr.assign(static_cast<const char*>(errB->GetBufferPointer()), errB->GetBufferSize());return false; }
		if (FAILED(D3DCompile(kPsSrc, strlen(kPsSrc), "PS", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &psB, &errB))) { if (errB && errB->GetBufferSize() && errB->GetBufferPointer())m_lastErr.assign(static_cast<const char*>(errB->GetBufferPointer()), errB->GetBufferSize());return false; }
		if (FAILED(dev->CreateVertexShader(vsB->GetBufferPointer(), vsB->GetBufferSize(), nullptr, &m_vs)))return false;
		if (FAILED(dev->CreatePixelShader(psB->GetBufferPointer(), psB->GetBufferSize(), nullptr, &m_ps)))return false;
		{
			ComPtr<ID3DBlob> rszB, rszErr;
			if (FAILED(D3DCompile(kPsResizeSrc, strlen(kPsResizeSrc), "RSZ", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &rszB, &rszErr))) { if (rszErr && rszErr->GetBufferSize() && rszErr->GetBufferPointer())m_lastErr.assign(static_cast<const char*>(rszErr->GetBufferPointer()), rszErr->GetBufferSize());return false; }
			if (FAILED(dev->CreatePixelShader(rszB->GetBufferPointer(), rszB->GetBufferSize(), nullptr, &m_rszPs)))return false;
		}
		D3D11_BLEND_DESC bd{};bd.RenderTarget[0].BlendEnable = TRUE;bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		if (FAILED(dev->CreateBlendState(&bd, &m_blend)))return false;
		D3D11_BUFFER_DESC cbd{};cbd.Usage = D3D11_USAGE_DYNAMIC;cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;cbd.ByteWidth = (UINT)((sizeof(EffectsCB) + 15) & ~15);
		if (FAILED(dev->CreateBuffer(&cbd, nullptr, &m_cb)))return false;
		D3D11_SAMPLER_DESC sd{};sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;sd.MaxLOD = D3D11_FLOAT32_MAX;
		if (FAILED(dev->CreateSamplerState(&sd, &m_samp)))return false;
		D3D11_RASTERIZER_DESC rd{};rd.FillMode = D3D11_FILL_SOLID;rd.CullMode = D3D11_CULL_NONE;rd.DepthClipEnable = FALSE;rd.ScissorEnable = TRUE;
		if (FAILED(dev->CreateRasterizerState(&rd, &m_rs)))return false;
		D3D11_DEPTH_STENCIL_DESC dsd{};dsd.DepthEnable = FALSE;dsd.StencilEnable = FALSE;
		if (FAILED(dev->CreateDepthStencilState(&dsd, &m_ds)))return false;
		return true;
	}
	void GenFallbackDepth(ID3D11Device* dev, ID3D11DeviceContext* ctx) { const int sz = 128;std::vector<float> fb(sz * sz);for (int y = 0;y < sz;++y) { float dv = static_cast<float>(y) / static_cast<float>(sz);for (int x = 0;x < sz;++x)fb[y * sz + x] = dv; }UpdateDepthTex(dev, ctx, fb, sz); }
	bool DownscaleFrame(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11Texture2D* src, UINT dstW, UINT dstH, RszSlot* slot = nullptr) {
		if (!dev || !ctx || !src || !m_rszPs)return false;
		ID3D11Texture2D* tex = nullptr;ID3D11RenderTargetView* rtv = nullptr;ID3D11ShaderResourceView* srv = nullptr;
		if (slot) {
			if (!slot->tex || slot->w != dstW || slot->h != dstH) {
				slot->tex.Reset();slot->rtv.Reset();slot->srv.Reset();
				D3D11_TEXTURE2D_DESC td{};
				td.Width = dstW;td.Height = dstH;td.MipLevels = 1;td.ArraySize = 1;
				td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;
				td.Usage = D3D11_USAGE_DEFAULT;
				td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				if (FAILED(dev->CreateTexture2D(&td, nullptr, &slot->tex)))return false;
				if (FAILED(dev->CreateRenderTargetView(slot->tex.Get(), nullptr, &slot->rtv)))return false;
				if (FAILED(dev->CreateShaderResourceView(slot->tex.Get(), nullptr, &slot->srv)))return false;
				slot->w = dstW;slot->h = dstH;
			}
			tex = slot->tex.Get();rtv = slot->rtv.Get();srv = slot->srv.Get();
		}
		else {
			if (!m_rszTex || m_rszW != dstW || m_rszH != dstH) {
				m_rszTex.Reset();m_rszRtv.Reset();m_rszSrv.Reset();
				D3D11_TEXTURE2D_DESC td{};
				td.Width = dstW;td.Height = dstH;td.MipLevels = 1;td.ArraySize = 1;
				td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;
				td.Usage = D3D11_USAGE_DEFAULT;
				td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				if (FAILED(dev->CreateTexture2D(&td, nullptr, &m_rszTex)))return false;
				if (FAILED(dev->CreateRenderTargetView(m_rszTex.Get(), nullptr, &m_rszRtv)))return false;
				if (FAILED(dev->CreateShaderResourceView(m_rszTex.Get(), nullptr, &m_rszSrv)))return false;
				m_rszW = dstW;m_rszH = dstH;
			}
			tex = m_rszTex.Get();rtv = m_rszRtv.Get();srv = m_rszSrv.Get();
		}
		D3D11_VIEWPORT vp{};vp.Width = (float)dstW;vp.Height = (float)dstH;vp.MaxDepth = 1.0f;
		ID3D11RenderTargetView* rtvs[] = { rtv };
		ID3D11ShaderResourceView* srvs[] = { m_scnSrv.Get() };
		ID3D11SamplerState* smps[] = { m_samp.Get() };
		ctx->OMSetRenderTargets(1, rtvs, nullptr);
		ctx->RSSetViewports(1, &vp);
		ctx->VSSetShader(m_vs.Get(), nullptr, 0);
		ctx->PSSetShader(m_rszPs.Get(), nullptr, 0);
		ctx->PSSetShaderResources(0, 1, srvs);
		ctx->PSSetSamplers(0, 1, smps);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetInputLayout(nullptr);
		ctx->Draw(3, 0);
		ID3D11ShaderResourceView* nullSrv[1] = { nullptr };
		ctx->PSSetShaderResources(0, 1, nullSrv);
		ID3D11RenderTargetView* nullRtv[1] = { nullptr };
		ctx->OMSetRenderTargets(1, nullRtv, nullptr);
		return true;
	}
	ID3D11Texture2D* GetDownscaled()const { return m_rszTex.Get(); }
	UINT DownscaleW()const { return m_rszW; }
	UINT DownscaleH()const { return m_rszH; }
	void UpdateSceneCropped(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11Texture2D* src, const RECT& r) {
		if (!dev || !ctx || !src)return;D3D11_TEXTURE2D_DESC sd{};src->GetDesc(&sd);LONG sW = static_cast<LONG>(sd.Width), sH = static_cast<LONG>(sd.Height);
		LONG cL = std::clamp(r.left, 0L, sW - 1);LONG cT = std::clamp(r.top, 0L, sH - 1);LONG cR = std::clamp(r.right, cL + 1, sW);LONG cB = std::clamp(r.bottom, cT + 1, sH);
		UINT cW = static_cast<UINT>(cR - cL), cH = static_cast<UINT>(cB - cT);
		if (!m_scnTex || !m_scnSrv || cW != m_scnW || cH != m_scnH) {
			D3D11_TEXTURE2D_DESC td = sd;td.Width = cW;td.Height = cH;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;td.Usage = D3D11_USAGE_DEFAULT;td.MiscFlags = 0;m_scnTex.Reset();m_scnSrv.Reset();
			std::vector<uint8_t> black((size_t)cW * cH * 4, 0);
			D3D11_SUBRESOURCE_DATA init{};
			init.pSysMem = black.data();
			init.SysMemPitch = cW * 4;
			if (FAILED(dev->CreateTexture2D(&td, &init, &m_scnTex)))return;D3D11_SHADER_RESOURCE_VIEW_DESC sv{};sv.Format = td.Format;sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;sv.Texture2D.MipLevels = 1;
			if (FAILED(dev->CreateShaderResourceView(m_scnTex.Get(), &sv, &m_scnSrv)))return;m_scnW = cW;m_scnH = cH;
		}
		D3D11_BOX box{ static_cast<UINT>(cL),static_cast<UINT>(cT),0,static_cast<UINT>(cR),static_cast<UINT>(cB),1 };ctx->CopySubresourceRegion(m_scnTex.Get(), 0, 0, 0, 0, src, 0, &box);
	}
	void UpdateDepthTex(ID3D11Device* dev, ID3D11DeviceContext* ctx, const std::vector<float>& depth, int sz) {
		if (!dev || !ctx || depth.empty() || sz <= 0)return;
		if (!m_depTex || sz != m_depSz) {
			D3D11_TEXTURE2D_DESC dd{};dd.Width = dd.Height = static_cast<UINT>(sz);dd.MipLevels = dd.ArraySize = 1;dd.Format = DXGI_FORMAT_R32_FLOAT;dd.SampleDesc.Count = 1;dd.Usage = D3D11_USAGE_DYNAMIC;dd.BindFlags = D3D11_BIND_SHADER_RESOURCE;dd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			m_depTex.Reset();m_depSrv.Reset();if (FAILED(dev->CreateTexture2D(&dd, nullptr, &m_depTex)))return;if (FAILED(dev->CreateShaderResourceView(m_depTex.Get(), nullptr, &m_depSrv)))return;m_depSz = sz;
		}
		D3D11_MAPPED_SUBRESOURCE mp{};if (SUCCEEDED(ctx->Map(m_depTex.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mp)) && mp.pData) { for (int y = 0;y < sz;++y)memcpy(static_cast<uint8_t*>(mp.pData) + static_cast<size_t>(y) * mp.RowPitch, &depth[static_cast<size_t>(y) * sz], static_cast<size_t>(sz) * sizeof(float));ctx->Unmap(m_depTex.Get(), 0); }
	}
	void UpdateMaskTex(ID3D11Device* dev, ID3D11DeviceContext* ctx, const std::vector<uint8_t>& mask8, int w, int h) {
		if (!dev || !ctx || mask8.empty() || w <= 0 || h <= 0)return;
		if (!m_maskTex || w != m_maskW || h != m_maskH) {
			D3D11_TEXTURE2D_DESC td{};
			td.Width = (UINT)w;td.Height = (UINT)h;td.MipLevels = 1;td.ArraySize = 1;
			td.Format = DXGI_FORMAT_R8_UNORM;td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DYNAMIC;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			m_maskTex.Reset();m_maskSrv.Reset();
			if (FAILED(dev->CreateTexture2D(&td, nullptr, &m_maskTex)))return;
			if (FAILED(dev->CreateShaderResourceView(m_maskTex.Get(), nullptr, &m_maskSrv)))return;
			m_maskW = w;m_maskH = h;
		}
		D3D11_MAPPED_SUBRESOURCE mp{};
		if (SUCCEEDED(ctx->Map(m_maskTex.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mp)) && mp.pData) {
			for (int y = 0;y < h;++y)memcpy(static_cast<uint8_t*>(mp.pData) + (size_t)y * mp.RowPitch, &mask8[(size_t)y * w], (size_t)w);
			ctx->Unmap(m_maskTex.Get(), 0);
		}
	}
	void ClearMask(ID3D11DeviceContext* ctx) {
		if (m_maskTex && ctx) { std::vector<uint8_t> z((size_t)m_maskW * m_maskH, 0);D3D11_MAPPED_SUBRESOURCE mp{};if (SUCCEEDED(ctx->Map(m_maskTex.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mp)) && mp.pData) { for (int y = 0;y < m_maskH;++y)memset(static_cast<uint8_t*>(mp.pData) + (size_t)y * mp.RowPitch, 0, (size_t)m_maskW);ctx->Unmap(m_maskTex.Get(), 0); } }
	}
	void UpdateStyleTex(ID3D11Device* dev, ID3D11DeviceContext* ctx, const std::vector<uint8_t>& rgba, int w, int h) {
		m_styleValid = false;
		if (!dev || !ctx || rgba.empty())return;
		if (!m_styleTex || w != m_styleW || h != m_styleH) {
			D3D11_TEXTURE2D_DESC td{};td.Width = (UINT)w;td.Height = (UINT)h;td.MipLevels = 1;td.ArraySize = 1;td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;td.Usage = D3D11_USAGE_DYNAMIC;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			m_styleTex.Reset();m_styleSrv.Reset();if (FAILED(dev->CreateTexture2D(&td, nullptr, &m_styleTex)))return;if (FAILED(dev->CreateShaderResourceView(m_styleTex.Get(), nullptr, &m_styleSrv)))return;m_styleW = w;m_styleH = h;
		}
		D3D11_MAPPED_SUBRESOURCE mp{};if (SUCCEEDED(ctx->Map(m_styleTex.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mp)) && mp.pData) { for (int y = 0;y < h;++y)memcpy(static_cast<uint8_t*>(mp.pData) + static_cast<size_t>(y) * mp.RowPitch, &rgba[static_cast<size_t>(y) * w * 4], static_cast<size_t>(w) * 4);ctx->Unmap(m_styleTex.Get(), 0);m_styleValid = true; }
	}
	void InvalidateStyle() { m_styleValid = false; }
	bool StyleValid()const { return m_styleValid; }
	void CaptureHistory(ID3D11Device* dev, ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv, UINT w, UINT h) {
		if (!m_histTex || w != m_histW || h != m_histH) {
			D3D11_TEXTURE2D_DESC td{};td.Width = w;td.Height = h;td.MipLevels = 1;td.ArraySize = 1;td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;td.Usage = D3D11_USAGE_DEFAULT;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			m_histTex.Reset();m_histSrv.Reset();if (FAILED(dev->CreateTexture2D(&td, nullptr, &m_histTex)))return;if (FAILED(dev->CreateShaderResourceView(m_histTex.Get(), nullptr, &m_histSrv)))return;m_histW = w;m_histH = h;
		}
		ComPtr<ID3D11Resource> res;rtv->GetResource(&res);ctx->CopyResource(m_histTex.Get(), res.Get());
	}
	void Render(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv, const EffectSettings& fx, const ShaderStyle* style, UINT vW, UINT vH, const D3D11_RECT& sc, float t, bool aiReady) {
		if (!ctx || !rtv || !m_scnSrv || !m_cb)return;ctx->OMSetRenderTargets(1, &rtv, nullptr);D3D11_VIEWPORT vp{};vp.Width = static_cast<float>(vW);vp.Height = static_cast<float>(vH);vp.MaxDepth = 1.0f;ctx->RSSetViewports(1, &vp);ctx->RSSetScissorRects(1, &sc);
		D3D11_MAPPED_SUBRESOURCE mp{};if (SUCCEEDED(ctx->Map(m_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mp)) && mp.pData) {
			EffectsCB cb{};
			cb.glossyIntensity = fx.glossyFloorIntensity;cb.glossyRoughness = fx.glossyRoughness;cb.glossyEnabled = fx.glossyEnabled ? 1.f : 0.f;cb.vibranceAmount = fx.vibranceAmount;
			cb.horizonY = fx.horizonY;cb.skyGlowStrength = fx.skyGlowStrength;cb.engineMode = static_cast<float>(fx.engineMode);cb.themeMode = static_cast<float>(fx.themeMode);
			cb.fogEnabled = fx.fogEnabled ? 1.f : 0.f;cb.viewportWidth = static_cast<float>(vW);cb.viewportHeight = static_cast<float>(vH);cb.textProtectEnabled = fx.textProtectEnabled ? 1.f : 0.f;
			cb.dofFocus = fx.dofFocusDistance;cb.dofBlurStrength = fx.dofBlurStrength;cb.dofEnabled = fx.dofEnabled ? 1.f : 0.f;cb.glossyFresnelPower = fx.glossyFresnelPower;
			cb.glossySpecularGlint = fx.glossySpecularGlint;cb.glossyContrast = fx.glossyContrast;cb.glossyTintR = fx.glossyTint[0];cb.glossyTintG = fx.glossyTint[1];cb.glossyTintB = fx.glossyTint[2];
			cb.clearCoatMode = fx.clearCoatMode ? 1.f : 0.f;cb.waveDistortionAmount = fx.waveDistortionAmount;cb.waveSpeed = fx.waveSpeed;cb.time = t;cb.bloomIntensity = fx.bloomIntensity;cb.bloomThreshold = fx.bloomThreshold;cb.chromaticAberration = fx.chromaticAberration;
			cb.vignetteIntensity = fx.vignetteIntensity;cb.vignetteSmoothness = fx.vignetteSmoothness;cb.filmGrainAmount = fx.filmGrainAmount;cb.sharpeningAmount = fx.sharpeningAmount;cb.exposure = fx.exposure;cb.fogDistance = fx.fogDistance;cb.contrastBoost = fx.contactShadow;cb.tempKelvin = fx.tempKelvin;
			cb.outlineEnabled = fx.outlineEnabled ? 1.f : 0.f;cb.outlineThickness = fx.outlineThickness;cb.outlineColorR = fx.outlineColor[0];cb.outlineColorG = fx.outlineColor[1];cb.outlineColorB = fx.outlineColor[2];
			cb.splitScreenPos = fx.splitScreenPos;cb.splitScreenEnabled = fx.splitScreenEnabled ? 1.f : 0.f;cb.specularTintR = fx.specularTint[0];cb.specularTintG = fx.specularTint[1];cb.specularTintB = fx.specularTint[2];
			cb.groundFadeEnd = fx.groundFadeEnd;cb.tonemapMode = static_cast<float>(fx.tonemapMode);cb.ssaoIntensity = fx.ssaoIntensity;cb.ssaoRadius = fx.ssaoRadius;cb.ssaoEnabled = fx.ssaoEnabled ? 1.f : 0.f;cb.dofRange = fx.dofRange;
			cb.fogDensity = fx.fogDensity;cb.fogColorR = fx.fogColor[0];cb.fogColorG = fx.fogColor[1];cb.fogColorB = fx.fogColor[2];cb.waveScale = fx.waveScale;cb.neonGlowIntensity = fx.neonGlowIntensity;cb.neonPinkBoost = fx.neonPinkBoost;cb.neonBlueFog = fx.neonBlueFog;
			cb.upscaleFSR = fx.upscaleFSR ? 1.f : 0.f;cb.upscaleDLSS = fx.upscaleDLSS ? 1.f : 0.f;cb.upscaleNAFNet = (fx.upscaleNAFNet && aiReady) ? 1.f : 0.f;
			cb.upscaleSD = (fx.upscaleSD && m_styleSrv && m_styleValid) ? 1.f : 0.f;
			cb.styleBlendStrength = std::clamp(fx.upscaleStrength, 0.f, 1.f);
			cb.upscaleTemporalSharpen = fx.upscaleTemporalSharpen ? 1.f : 0.f;cb.upscaleQuality = static_cast<float>(fx.upscaleQuality);cb.sharpeningRadius = fx.sharpeningRadius;cb.fxaaEnabled = fx.fxaaEnabled ? 1.f : 0.f;
			cb.godRayIntensity = fx.godRayIntensity;cb.godRayDecay = std::clamp(fx.godRayDecay, 0.5f, 0.99f);cb.motionBlurAmount = fx.motionBlurAmount;cb.godRayX = fx.godRayX;cb.godRayY = fx.godRayY;cb.fsrSharpness = fx.fsrSharpness;
			cb.styleAspect = (m_styleW > 0 && m_styleH > 0) ? (float)m_styleW / (float)m_styleH : 1.0f;
			cb.scanlineIntensity = fx.scanlineIntensity;cb.crtCurve = fx.crtCurve;cb.glitchAmount = fx.glitchAmount;cb.zoomBlur = fx.zoomBlur;cb.tiltShift = fx.tiltShift;cb.panini = fx.panini;cb.sunAngle = fx.sunAngle;cb.ambientLight = fx.ambientLight;
			cb.rainDrops = fx.rainDrops;cb.windDistortion = fx.windDistortion;cb.snowAmount = fx.snowAmount;cb.nightVision = fx.nightVision;cb.thermalVision = fx.thermalVision;
			cb.shadowTintR = fx.shadowTint[0];cb.shadowTintG = fx.shadowTint[1];cb.shadowTintB = fx.shadowTint[2];cb.matRoughness = fx.matRoughness;cb.matMetalness = fx.matMetalness;cb.bloomSpectrum = fx.bloomSpectrum;cb.lensDirt = fx.lensDirt;
			memcpy(cb.customFx, fx.customFx, sizeof(cb.customFx));
			if (style) {
				cb.styleToon = style->toonLevels;cb.styleRim = style->rimLighting;cb.stylePalette = style->paletteMix;cb.styleEdge = style->edgeSoftness;
				cb.styleShadowTintR = style->shadowTint[0];cb.styleShadowTintG = style->shadowTint[1];cb.styleShadowTintB = style->shadowTint[2];
				cb.styleHighlightTintR = style->highlightTint[0];cb.styleHighlightTintG = style->highlightTint[1];cb.styleHighlightTintB = style->highlightTint[2];
				cb.styleWarmCool = style->warmCool;cb.styleNeon = style->neonGlow;
			}
			else {
				cb.styleToon = 0;cb.styleRim = 0;cb.stylePalette = 0;cb.styleEdge = 0;
				cb.styleShadowTintR = 0.05f;cb.styleShadowTintG = 0.05f;cb.styleShadowTintB = 0.1f;
				cb.styleHighlightTintR = 1;cb.styleHighlightTintG = 1;cb.styleHighlightTintB = 1;
				cb.styleWarmCool = 0;cb.styleNeon = 0;
			}
			memcpy(mp.pData, &cb, sizeof(cb));ctx->Unmap(m_cb.Get(), 0);
		}
		ID3D11ShaderResourceView* fallback = m_scnSrv.Get();ID3D11ShaderResourceView* srvs[] = { m_scnSrv.Get(),m_depSrv.Get(),m_styleSrv ? m_styleSrv.Get() : fallback,m_histSrv ? m_histSrv.Get() : fallback,m_maskSrv ? m_maskSrv.Get() : fallback };
		ID3D11SamplerState* smps[] = { m_samp.Get() };ID3D11Buffer* cbs[] = { m_cb.Get() };
		ctx->VSSetShader(m_vs.Get(), nullptr, 0);ctx->PSSetShader(m_ps.Get(), nullptr, 0);ctx->PSSetShaderResources(0, 5, srvs);ctx->PSSetSamplers(0, 1, smps);ctx->PSSetConstantBuffers(0, 1, cbs);
		ctx->RSSetState(m_rs.Get());ctx->OMSetDepthStencilState(m_ds.Get(), 0);ctx->OMSetBlendState(m_blend.Get(), nullptr, 0xffffffff);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);ctx->IASetInputLayout(nullptr);ctx->Draw(3, 0);
		ID3D11ShaderResourceView* nullSrvs[5] = { nullptr,nullptr,nullptr,nullptr,nullptr };ctx->PSSetShaderResources(0, 5, nullSrvs);ID3D11RenderTargetView* nullRtv[1] = { nullptr };ctx->OMSetRenderTargets(1, nullRtv, nullptr);
	}
	ID3D11Texture2D* GetScene()const { return m_scnTex.Get(); }std::string LastError()const { return m_lastErr; }
private:
	std::string m_lastErr;ComPtr<ID3D11VertexShader> m_vs;ComPtr<ID3D11PixelShader> m_ps;ComPtr<ID3D11PixelShader> m_rszPs;ComPtr<ID3D11Buffer> m_cb;ComPtr<ID3D11SamplerState> m_samp;ComPtr<ID3D11BlendState> m_blend;ComPtr<ID3D11RasterizerState> m_rs;ComPtr<ID3D11DepthStencilState> m_ds;
	ComPtr<ID3D11Texture2D> m_scnTex;ComPtr<ID3D11ShaderResourceView> m_scnSrv;UINT m_scnW = 0, m_scnH = 0;ComPtr<ID3D11Texture2D> m_rszTex;ComPtr<ID3D11RenderTargetView> m_rszRtv;ComPtr<ID3D11ShaderResourceView> m_rszSrv;UINT m_rszW = 0, m_rszH = 0;ComPtr<ID3D11Texture2D> m_depTex;ComPtr<ID3D11ShaderResourceView> m_depSrv;int m_depSz = 0;
	ComPtr<ID3D11Texture2D> m_styleTex;ComPtr<ID3D11ShaderResourceView> m_styleSrv;int m_styleW = 0, m_styleH = 0;bool m_styleValid = false;ComPtr<ID3D11Texture2D> m_maskTex;ComPtr<ID3D11ShaderResourceView> m_maskSrv;int m_maskW = 0, m_maskH = 0;ComPtr<ID3D11Texture2D> m_histTex;ComPtr<ID3D11ShaderResourceView> m_histSrv;UINT m_histW = 0, m_histH = 0;
};
class CompositingPipeline;
static bool CaptureSceneToBgra(CompositingPipeline& comp, std::vector<uint8_t>& out, int& w, int& h) {
	ID3D11Texture2D* scene = comp.GetScene();
	if (!scene || !g_dev || !g_ctx)return false;
	D3D11_TEXTURE2D_DESC sd{};
	scene->GetDesc(&sd);
	static ComPtr<ID3D11Texture2D> staging;
	static UINT sw = 0, sh = 0;
	if (!staging || sw != sd.Width || sh != sd.Height) {
		staging.Reset();
		D3D11_TEXTURE2D_DESC st = sd;
		st.Usage = D3D11_USAGE_STAGING;
		st.BindFlags = 0;
		st.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		st.MiscFlags = 0;
		if (FAILED(g_dev->CreateTexture2D(&st, nullptr, &staging)))return false;
		sw = sd.Width;sh = sd.Height;
	}
	g_ctx->CopyResource(staging.Get(), scene);
	D3D11_MAPPED_SUBRESOURCE mp{};
	if (FAILED(g_ctx->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mp)) || !mp.pData)return false;
	w = (int)sd.Width;h = (int)sd.Height;
	out.resize((size_t)w * h * 4);
	for (int y = 0;y < h;++y)
		memcpy(&out[(size_t)y * w * 4], (const uint8_t*)mp.pData + (size_t)y * mp.RowPitch, (size_t)w * 4);
	g_ctx->Unmap(staging.Get(), 0);
	return true;
}
#if SR_HAS_ONNX
class SimpleUpscaler {
public:
	bool Init(std::string& err) {
		if (m_ready)return true;
		m_err.clear();
		if (!ValidateModelFile(L"models\\real_esrgan_x4plus-onnx-float\\real_esrgan_x4plus.onnx", 400000) ||
			!ValidateModelFile(L"models\\real_esrgan_x4plus-onnx-float\\real_esrgan_x4plus.data", 40000000)) {
			m_err = "upscaler model missing";
			return false;
		}
		bool anyGpu = false;
		Ep eps[] = { EpCuda,EpDml };
		for (Ep ep : eps) {
			try {
				Ort::SessionOptions opts;
				opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				opts.SetIntraOpNumThreads(0);
				opts.DisableMemPattern();
				if (!AppendEpByName(opts, ep))continue;
				m_sess = Ort::Session(GetOrtEnv(), L"models\\real_esrgan_x4plus-onnx-float\\real_esrgan_x4plus.onnx", opts);
				m_onGpu = true;
				anyGpu = true;
				break;
			}
			catch (const std::exception&) {}
		}
		if (!anyGpu) {
			try {
				Ort::SessionOptions copts;
				copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				copts.SetIntraOpNumThreads(0);
				m_sess = Ort::Session(GetOrtEnv(), L"models\\real_esrgan_x4plus-onnx-float\\real_esrgan_x4plus.onnx", copts);
			}
			catch (const std::exception& e2) {
				m_err = std::string("upscaler load failed: ") + e2.what();
				return false;
			}
		}
		m_in = OrtSessionInputName(m_sess, 0);
		m_out = OrtSessionOutputName(m_sess, 0);
		m_ready = true;
		return true;
	}
	bool IsReady()const { return m_ready; }
	bool OnGpu()const { return m_onGpu; }
	std::string LastErr()const { return m_err; }
	bool Run(const std::vector<uint8_t>& bgra, int w, int h, std::vector<uint8_t>& out, int& ow, int& oh, double* msOut) {
		if (!m_ready || !m_sess)return false;
		auto t0 = std::chrono::steady_clock::now();
		try {
			const int IW = 128, IH = 128;
			float sc = std::min((float)IW / (float)w, (float)IH / (float)h);
			int fw = std::max(1, std::min(IW, (int)(w * sc + 0.5f)));
			int fh = std::max(1, std::min(IH, (int)(h * sc + 0.5f)));
			int ox = (IW - fw) / 2, oy = (IH - fh) / 2;
			std::vector<float> input((size_t)3 * IW * IH);
			for (int y = 0;y < IH;++y) {
				float sy = ((float)(y - oy) + 0.5f) * (float)h / (float)fh - 0.5f;
				sy = std::clamp(sy, 0.0f, (float)(h - 1));
				int y0 = (int)sy, y1 = std::min(y0 + 1, h - 1);
				float fy = sy - (float)y0;
				for (int x = 0;x < IW;++x) {
					float sx = ((float)(x - ox) + 0.5f) * (float)w / (float)fw - 0.5f;
					sx = std::clamp(sx, 0.0f, (float)(w - 1));
					int x0 = (int)sx, x1 = std::min(x0 + 1, w - 1);
					float fx = sx - (float)x0;
					auto px = [&](int xx, int yy)-> std::array<float, 3> {
						const uint8_t* p = &bgra[((size_t)yy * w + xx) * 4];
						return { p[2] / 255.0f,p[1] / 255.0f,p[0] / 255.0f };
						};
					auto a = px(x0, y0), b = px(x1, y0), cc = px(x0, y1), d = px(x1, y1);
					size_t idx = (size_t)y * IW + x;
					for (int ch = 0;ch < 3;++ch) {
						float top = a[ch] + (b[ch] - a[ch]) * fx;
						float bot = cc[ch] + (d[ch] - cc[ch]) * fx;
						input[ch * (size_t)IW * IH + idx] = top + (bot - top) * fy;
					}
				}
			}
			std::vector<float> big;
			if (!Exec128(input, big, m_err))return false;
			ow = fw * 4;oh = fh * 4;
			out.resize((size_t)ow * oh * 4);
			const size_t plane = (size_t)512 * 512;
			for (int y = 0;y < oh;++y)
				for (int x = 0;x < ow;++x) {
					size_t tp = (size_t)(oy * 4 + y) * 512 + (ox * 4 + x);
					float tr = big[tp], tg = big[plane + tp], tb = big[2 * plane + tp];
					uint8_t* d = &out[((size_t)y * ow + x) * 4];
					d[0] = (uint8_t)(std::clamp(tb, 0.0f, 1.0f) * 255.0f + 0.5f);
					d[1] = (uint8_t)(std::clamp(tg, 0.0f, 1.0f) * 255.0f + 0.5f);
					d[2] = (uint8_t)(std::clamp(tr, 0.0f, 1.0f) * 255.0f + 0.5f);
					d[3] = 255;
				}
			if (msOut)*msOut = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
			return true;
		}
		catch (const std::exception& e) {
			m_err = std::string("upscaler run failed: ") + e.what();
			return false;
		}
	}
	bool RunTiled(const std::vector<uint8_t>& bgra, int w, int h, std::vector<uint8_t>& out, int& ow, int& oh, double* msOut) {
		if (!m_ready || !m_sess)return false;
		auto t0 = std::chrono::steady_clock::now();
		try {
			const int IW = 128, IH = 128, STR = 112, FE = 8;
			if (w <= 0 || h <= 0) { m_err = "upscaler run failed: bad input size";return false; }
			ow = w * 4;oh = h * 4;
			out.assign((size_t)ow * oh * 4, 0);
			std::vector<float> acc((size_t)ow * oh * 4, 0.0f);
			std::vector<float> wt((size_t)ow * oh, 0.0f);
			int ncx = std::max(1, (int)std::ceil((float)(w - IW) / (float)STR) + 1);
			int ncy = std::max(1, (int)std::ceil((float)(h - IH) / (float)STR) + 1);
			int maxX0 = std::max(0, w - IW), maxY0 = std::max(0, h - IH);
			struct TileJob { int x0, y0;std::vector<float> data; };
			std::vector<TileJob> jobs;
			jobs.reserve((size_t)ncx * ncy);
			for (int ty = 0;ty < ncy;++ty) {
				int y0 = std::min(ty * STR, maxY0);
				for (int tx = 0;tx < ncx;++tx)
					jobs.push_back({ std::min(tx * STR,maxX0),y0,{} });
			}
			std::atomic<size_t> next{ 0 };
			std::atomic<bool> fail{ false };
			auto tileWorker = [&]() {
				for (;;) {
					size_t idx = next.fetch_add(1);
					if (idx >= jobs.size())break;
					TileJob& j = jobs[idx];
					try {
						std::vector<float> input((size_t)3 * IW * IH);
						for (int y = 0;y < IH;++y) {
							int iy = std::clamp(j.y0 + y, 0, h - 1);
							for (int x = 0;x < IW;++x) {
								int ix = std::clamp(j.x0 + x, 0, w - 1);
								const uint8_t* p = &bgra[((size_t)iy * w + ix) * 4];
								size_t id = (size_t)y * IW + x;
								input[id] = p[2] / 255.0f;
								input[(size_t)IW * IH + id] = p[1] / 255.0f;
								input[(size_t)2 * IW * IH + id] = p[0] / 255.0f;
							}
						}
						std::string te;
						if (!Exec128(input, j.data, te)) { fail.store(true);return; }
					}
					catch (...) { fail.store(true);return; }
				}
				};
			size_t nt = std::min<size_t>(jobs.size(), (size_t)std::max(1u, std::thread::hardware_concurrency()));
			if (nt > 1) {
				std::vector<std::thread> pool;
				pool.reserve(nt - 1);
				for (size_t ti = 1;ti < nt;++ti)pool.emplace_back(tileWorker);
				tileWorker();
				for (auto& th : pool)th.join();
			}
			else {
				tileWorker();
			}
			if (fail.load()) { m_err = "upscaler tile run failed";return false; }
			const size_t plane = (size_t)512 * 512;
			for (auto& j : jobs) {
				int dstX0 = j.x0 * 4, dstY0 = j.y0 * 4;
				int dstX1 = std::min(dstX0 + IW * 4, ow);
				int dstY1 = std::min(dstY0 + IH * 4, oh);
				for (int oy = dstY0;oy < dstY1;++oy) {
					float ly = (float)(oy - dstY0) / 4.0f;
					float wy = std::clamp(std::min(ly, (float)(IH - 1) - ly) / (float)FE, 0.0f, 1.0f);
					float wyf = std::max(wy, 0.01f);
					for (int oxx = dstX0;oxx < dstX1;++oxx) {
						float lx = (float)(oxx - dstX0) / 4.0f;
						float wx = std::clamp(std::min(lx, (float)(IW - 1) - lx) / (float)FE, 0.0f, 1.0f);
						float wgt = std::max(wx * wyf, 0.01f);
						size_t tp = (size_t)(oy - dstY0) * 512 + (oxx - dstX0);
						float tr = j.data[tp], tg = j.data[plane + tp], tb = j.data[2 * plane + tp];
						size_t oi = (size_t)oy * ow + oxx;
						float* d = &acc[oi * 4];
						d[0] += tb * wgt;d[1] += tg * wgt;d[2] += tr * wgt;
						wt[oi] += wgt;
					}
				}
			}
			for (size_t i = 0;i < (size_t)ow * oh;++i) {
				float wsum = wt[i];
				if (wsum <= 0.0f)wsum = 1.0f;
				uint8_t* d = &out[i * 4];
				const float* a = &acc[i * 4];
				d[0] = (uint8_t)(std::clamp(a[0] / wsum, 0.0f, 1.0f) * 255.0f + 0.5f);
				d[1] = (uint8_t)(std::clamp(a[1] / wsum, 0.0f, 1.0f) * 255.0f + 0.5f);
				d[2] = (uint8_t)(std::clamp(a[2] / wsum, 0.0f, 1.0f) * 255.0f + 0.5f);
				d[3] = 255;
			}
			if (msOut)*msOut = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
			return true;
		}
		catch (const std::exception& e) {
			m_err = std::string("upscaler run failed: ") + e.what();
			return false;
		}
	}
private:
	bool Exec128(const std::vector<float>& input, std::vector<float>& out512, std::string& errOut) {
		Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
		std::array<int64_t, 4> shape{ 1,3,128,128 };
		Ort::Value inT = Ort::Value::CreateTensor<float>(mi, const_cast<float*>(input.data()), input.size(), shape.data(), shape.size());
		const char* inN[] = { m_in.c_str() };
		const char* outN[] = { m_out.c_str() };
		auto res = m_sess.Run(Ort::RunOptions{ nullptr }, inN, &inT, 1, outN, 1);
		size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
		if (cnt < (size_t)512 * 512 * 3) { errOut = "upscaler output too small";return false; }
		float* raw = res[0].GetTensorMutableData<float>();
		out512.assign(raw, raw + (size_t)512 * 512 * 3);
		return true;
	}
	Ort::Session m_sess{ nullptr };
	std::string m_in, m_out, m_err;
	bool m_ready = false, m_onGpu = false;
};
#else
class SimpleUpscaler {
public:
	bool Init(std::string& err) { err = "not built with ONNX";return false; }
	bool IsReady()const { return false; }
	bool OnGpu()const { return false; }
	std::string LastErr()const { return "not built with ONNX"; }
	bool Run(const std::vector<uint8_t>&, int, int, std::vector<uint8_t>&, int&, int&, double*) { return false; }
	bool RunTiled(const std::vector<uint8_t>&, int, int, std::vector<uint8_t>&, int&, int&, double*) { return false; }
};
#endif
static SimpleUpscaler g_sdUpscaler;
static void g_sdUpscalerLoad() { std::string e;if (!g_sdUpscaler.IsReady())g_sdUpscaler.Init(e); }
static bool g_sdUpscalerRun(std::vector<uint8_t>& img, int w, int h, std::vector<uint8_t>& out, int& ow, int& oh, double* ms) {
	try {
		if (!g_sdUpscaler.IsReady()) { g_sdUpscalerLoad(); }
		return g_sdUpscaler.Run(img, w, h, out, ow, oh, ms);
	}
	catch (const std::exception&) {
		return false;
	}
	catch (...) {
		return false;
	}
}
static bool g_sdUpscalerRunTiled(std::vector<uint8_t>& img, int w, int h, std::vector<uint8_t>& out, int& ow, int& oh, double* ms) {
	try {
		if (!g_sdUpscaler.IsReady()) { g_sdUpscalerLoad(); }
		return g_sdUpscaler.RunTiled(img, w, h, out, ow, oh, ms);
	}
	catch (const std::exception&) {
		return false;
	}
	catch (...) {
		return false;
	}
}
static std::string g_sdUpscalerErr() { return g_sdUpscaler.LastErr(); }
#if SR_HAS_ONNX
class MoebiusEngine {
public:
	bool Init(std::string& err) {
		m_err.clear();
		if (m_ready)return true;
		if (!MoebiusReady()) { m_err = "moebius model files missing";return false; }
		try {
			m_unet = LoadSession(L"models\\moebius\\unet.onnx", m_unetGpu);
			m_enc = LoadSession(L"models\\moebius\\vae_encoder.onnx", m_encGpu);
			m_dec = LoadSession(L"models\\moebius\\vae_decoder.onnx", m_decGpu);
		}
		catch (const std::exception& e) {
			m_err = std::string("moebius load failed: ") + e.what();
			return false;
		}
		m_encIn = OrtSessionInputName(m_enc, 0);m_encOut = OrtSessionOutputName(m_enc, 0);
		m_decIn = OrtSessionInputName(m_dec, 0);m_decOut = OrtSessionOutputName(m_dec, 0);
		m_unetIns.clear();
		for (int i = 0;i < (int)m_unet.GetInputCount();++i) {
			MbIn in;
			in.name = OrtSessionInputName(m_unet, i);
			try { in.type = m_unet.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType(); }
			catch (...) { in.type = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED; }
			m_unetIns.push_back(in);
		}
		m_unetOut = OrtSessionOutputName(m_unet, 0);
		m_ready = true;
		return true;
	}
	bool IsReady()const { return m_ready; }
	std::string LastErr()const { return m_err; }
	bool OnGpu()const { return m_unetGpu && m_encGpu && m_decGpu; }
	void SetCpuOnly() { m_cpuOnly = true; }
	bool CpuOnly()const { return m_cpuOnly; }
	bool Generate(const std::vector<uint8_t>& bgra, int fw, int fh, int steps, unsigned seed,
		std::vector<uint8_t>& out, int& ow, int& oh, double* msOut, std::string& errOut,
		const std::atomic<bool>* cancel = nullptr) {
		if (!m_ready) { errOut = "Moebius not loaded.";return false; }
		auto t0 = std::chrono::steady_clock::now();
		try {
			const int IMG = 512, LAT = 64, NUM_T = 1000;
			const float NOISE_OFF = 0.0357f, GUID = 4.5f;
			if (fw <= 0 || fh <= 0) { errOut = "bad input";return false; }
			steps = std::clamp(steps, 1, 8);
			const size_t plane = (size_t)LAT * LAT;
			const size_t n = 4 * plane;
			std::vector<uint8_t> sq((size_t)IMG * IMG * 4, 0);
			float sc2 = std::min((float)IMG / (float)fw, (float)IMG / (float)fh);
			int iw = std::max(1, std::min(IMG, (int)(fw * sc2)));
			int ih = std::max(1, std::min(IMG, (int)(fh * sc2)));
			int ox = (IMG - iw) / 2, oy = (IMG - ih) / 2;
			for (int y = 0;y < ih;++y) {
				float sy = ((float)y + 0.5f) * (float)fh / (float)ih - 0.5f;
				sy = std::clamp(sy, 0.0f, (float)(fh - 1));
				int y0 = (int)sy, y1 = std::min(y0 + 1, fh - 1);
				float fy = sy - (float)y0;
				for (int x = 0;x < iw;++x) {
					float sx = ((float)x + 0.5f) * (float)fw / (float)iw - 0.5f;
					sx = std::clamp(sx, 0.0f, (float)(fw - 1));
					int x0 = (int)sx, x1 = std::min(x0 + 1, fw - 1);
					float fx = sx - (float)x0;
					uint8_t* d = &sq[((size_t)(oy + y) * IMG + (ox + x)) * 4];
					for (int c = 0;c < 3;++c) {
						const uint8_t* p00 = &bgra[((size_t)y0 * fw + x0) * 4];
						const uint8_t* p01 = &bgra[((size_t)y0 * fw + x1) * 4];
						const uint8_t* p10 = &bgra[((size_t)y1 * fw + x0) * 4];
						const uint8_t* p11 = &bgra[((size_t)y1 * fw + x1) * 4];
						float top = p00[c] * (1.0f - fx) + p01[c] * fx;
						float bot = p10[c] * (1.0f - fx) + p11[c] * fx;
						d[c] = (uint8_t)(top * (1.0f - fy) + bot * fy + 0.5f);
					}
					d[3] = 255;
				}
			}
			std::vector<float> maskedLat;
			if (!Encode(sq, maskedLat, errOut))return false;
			std::vector<float> latents(n), eps(n), next(n);
			std::mt19937 rng(seed);
			std::normal_distribution<float> nd(0.0f, 1.0f);
			std::mt19937 rng2(seed ^ 0x9e3779b9u);
			float offv[4];
			for (int c = 0;c < 4;++c)offv[c] = nd(rng2);
			for (size_t i = 0;i < n;++i)latents[i] = nd(rng) + NOISE_OFF * offv[i / plane];
			std::vector<float> ac(NUM_T);
			double accA = 1.0;
			float a0 = std::sqrt(0.00085f), b0 = std::sqrt(0.012f);
			for (int i = 0;i < NUM_T;++i) {
				float s = a0 + (b0 - a0) * ((float)i / (float)(NUM_T - 1));
				accA *= 1.0 - (double)(s * s);
				ac[i] = (float)accA;
			}
			int stepRatio = NUM_T / steps;
			std::vector<int> ts;
			ts.reserve(steps);
			for (int i = 0;i < steps;++i)ts.push_back(i * stepRatio);
			std::reverse(ts.begin(), ts.end());
			for (size_t si = 0;si < ts.size();++si) {
				if (cancel && cancel->load()) { errOut = "cancelled";return false; }
				int t = ts[si];
				int prevT = (si + 1 < ts.size()) ? ts[si + 1] : -1;
				if (!UnetCFG(latents, maskedLat, t, eps, errOut))return false;
				float acT = ac[(size_t)t], acPrev = prevT >= 0 ? ac[(size_t)prevT] : 1.0f;
				float sqA = std::sqrt(acT), sqB = std::sqrt(1.0f - acT);
				float sqP = std::sqrt(acPrev), sqQ = std::sqrt(1.0f - acPrev);
				for (size_t i = 0;i < n;++i) {
					float predX0 = (latents[i] - sqB * eps[i]) / sqA;
					next[i] = sqP * predX0 + sqQ * eps[i];
				}
				latents.swap(next);
			}
			std::vector<uint8_t> sqOut;
			if (!Decode(latents, sqOut, errOut))return false;
			int bandH = (int)(IMG * 9 / 16);
			int bandY = (IMG - bandH) / 2;
			ow = IMG;oh = bandH;
			out.resize((size_t)IMG * bandH * 4);
			for (int y = 0;y < bandH;++y)
				memcpy(&out[(size_t)y * IMG * 4], &sqOut[((size_t)(bandY + y) * IMG) * 4], (size_t)IMG * 4);
			if (msOut)*msOut = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
			return true;
		}
		catch (const std::exception& e) {
			errOut = std::string("moebius failed: ") + e.what();
			return false;
		}
	}
private:
	Ort::Session LoadSession(const wchar_t* path, bool& gpuFlag) {
		gpuFlag = false;
		if (!m_cpuOnly) {
			Ep eps[] = { EpCuda,EpDml };
			for (Ep ep : eps) {
				try {
					Ort::SessionOptions bopts;
					bopts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
					bopts.SetIntraOpNumThreads(0);
					bopts.DisableMemPattern();
					try { bopts.AddConfigEntry("dml_disable_metacommands", "1"); }
					catch (...) {}
					if (!AppendEpByName(bopts, ep))throw std::runtime_error("provider not in build");
					Ort::Session s = Ort::Session(GetOrtEnv(), path, bopts);
					gpuFlag = true;
					return s;
				}
				catch (const std::exception& e) {
					m_err = std::string(ep == EpCuda ? "cuda: " : "dml: ") + e.what();
				}
			}
			try {
				Ort::SessionOptions opts;
				opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				opts.SetIntraOpNumThreads(0);
				if (!AppendEpByName(opts, EpDml))throw std::runtime_error("provider not in build");
				Ort::Session s = Ort::Session(GetOrtEnv(), path, opts);
				gpuFlag = true;
				return s;
			}
			catch (const std::exception& e) {
				m_err = std::string("dml2: ") + e.what();
			}
		}
		Ort::SessionOptions copts;
		copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
		copts.SetIntraOpNumThreads(0);
		return Ort::Session(GetOrtEnv(), path, copts);
	}
	bool Encode(const std::vector<uint8_t>& sq, std::vector<float>& lat, std::string& errOut) {
		const int IMG = 512, LAT = 64;
		std::vector<float> chw((size_t)3 * IMG * IMG);
		for (int y = 0;y < IMG;++y)
			for (int x = 0;x < IMG;++x) {
				const uint8_t* p = &sq[((size_t)y * IMG + x) * 4];
				size_t idx = (size_t)y * IMG + x;
				chw[idx] = p[2] / 255.0f;
				chw[(size_t)IMG * IMG + idx] = p[1] / 255.0f;
				chw[(size_t)2 * IMG * IMG + idx] = p[0] / 255.0f;
			}
		Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
		std::array<int64_t, 4> sh{ 1,3,IMG,IMG };
		Ort::Value v = Ort::Value::CreateTensor<float>(mi, chw.data(), chw.size(), sh.data(), sh.size());
		const char* inN[] = { m_encIn.c_str() };
		const char* outN[] = { m_encOut.c_str() };
		auto res = m_enc.Run(Ort::RunOptions{ nullptr }, inN, &v, 1, outN, 1);
		float* raw = res[0].GetTensorMutableData<float>();
		size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
		if (cnt < 8 * (size_t)LAT * LAT) { errOut = "moebius vae encoder output too small";return false; }
		lat.resize(4 * (size_t)LAT * LAT);
		for (size_t i = 0;i < lat.size();++i)lat[i] = raw[i] * 0.13025f;
		return true;
	}
	bool Decode(const std::vector<float>& lat, std::vector<uint8_t>& sq, std::string& errOut) {
		const int IMG = 512;
		std::vector<float> scaled(lat.size());
		for (size_t i = 0;i < lat.size();++i)scaled[i] = lat[i] / 0.13025f;
		Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
		std::array<int64_t, 4> sh{ 1,4,64,64 };
		Ort::Value v = Ort::Value::CreateTensor<float>(mi, scaled.data(), scaled.size(), sh.data(), sh.size());
		const char* inN[] = { m_decIn.c_str() };
		const char* outN[] = { m_decOut.c_str() };
		auto res = m_dec.Run(Ort::RunOptions{ nullptr }, inN, &v, 1, outN, 1);
		float* raw = res[0].GetTensorMutableData<float>();
		size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
		if (cnt < 3 * (size_t)IMG * IMG) { errOut = "moebius vae decoder output too small";return false; }
		sq.resize((size_t)IMG * IMG * 4);
		for (int y = 0;y < IMG;++y)
			for (int x = 0;x < IMG;++x) {
				size_t idx = (size_t)y * IMG + x;
				uint8_t* d = &sq[idx * 4];
				d[0] = (uint8_t)(std::clamp(raw[2 * (size_t)IMG * IMG + idx], 0.0f, 1.0f) * 255.0f + 0.5f);
				d[1] = (uint8_t)(std::clamp(raw[1 * (size_t)IMG * IMG + idx], 0.0f, 1.0f) * 255.0f + 0.5f);
				d[2] = (uint8_t)(std::clamp(raw[0 * (size_t)IMG * IMG + idx], 0.0f, 1.0f) * 255.0f + 0.5f);
				d[3] = 255;
			}
		return true;
	}
	bool UnetCFG(const std::vector<float>& latents, const std::vector<float>& maskedLat, int t,
		std::vector<float>& out, std::string& errOut) {
		const size_t plane = (size_t)64 * 64;
		std::vector<float> nine(9 * plane), nine2(2 * 9 * plane);
		memcpy(nine.data(), latents.data(), 4 * plane * sizeof(float));
		for (size_t i = 0;i < plane;++i)nine[4 * plane + i] = 1.0f;
		memcpy(&nine[5 * plane], maskedLat.data(), 4 * plane * sizeof(float));
		memcpy(nine2.data(), nine.data(), 9 * plane * sizeof(float));
		memcpy(&nine2[9 * plane], nine.data(), 9 * plane * sizeof(float));
		std::vector<int64_t> ids(2 * 10);
		for (int i = 0;i < 10;++i) { ids[i] = 10 + i;ids[10 + i] = i; }
		int64_t tsv[2] = { t,t };
		Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
		std::array<int64_t, 4> sh9{ 2,9,64,64 };
		std::array<int64_t, 1> shT{ 2 };
		std::array<int64_t, 2> shI{ 2,10 };
		std::vector<const char*> inNames;
		std::vector<Ort::Value> inVals;
		for (auto& in : m_unetIns) {
			const std::string& nm = in.name;
			if (nm.find("timestep") != std::string::npos) {
				inNames.push_back(nm.c_str());
				inVals.emplace_back(Ort::Value::CreateTensor<int64_t>(mi, tsv, 2, shT.data(), shT.size()));
			}
			else if (nm.find("input_id") != std::string::npos || nm.find("ids") != std::string::npos) {
				inNames.push_back(nm.c_str());
				inVals.emplace_back(Ort::Value::CreateTensor<int64_t>(mi, ids.data(), ids.size(), shI.data(), shI.size()));
			}
			else {
				inNames.push_back(nm.c_str());
				inVals.emplace_back(Ort::Value::CreateTensor<float>(mi, nine2.data(), nine2.size(), sh9.data(), sh9.size()));
			}
		}
		const char* outN[] = { m_unetOut.c_str() };
		auto res = m_unet.Run(Ort::RunOptions{ nullptr }, inNames.data(), inVals.data(), (int)inVals.size(), outN, 1);
		float* raw = res[0].GetTensorMutableData<float>();
		size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
		if (cnt < 2 * 4 * plane) { errOut = "moebius unet output too small";return false; }
		out.resize(4 * plane);
		for (size_t i = 0;i < 4 * plane;++i) {
			float u = raw[i], c = raw[4 * plane + i];
			out[i] = u + 4.5f * (c - u);
		}
		return true;
	}
public:
	void Release() {
		m_enc = Ort::Session{ nullptr };
		m_dec = Ort::Session{ nullptr };
		m_unet = Ort::Session{ nullptr };
		m_ready = false;
		m_encGpu = m_decGpu = m_unetGpu = false;
	}
	struct MbIn { std::string name;ONNXTensorElementDataType type = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED; };
	Ort::Session m_enc{ nullptr }, m_dec{ nullptr }, m_unet{ nullptr };
	std::string m_encIn, m_encOut, m_decIn, m_decOut;
	std::vector<MbIn> m_unetIns;
	std::string m_unetOut;
	std::string m_err;
	bool m_ready = false;
	bool m_cpuOnly = false;
	bool m_encGpu = false, m_decGpu = false, m_unetGpu = false;
};
#else
class MoebiusEngine {
public:
	bool Init(std::string& err) { err = "not built with ONNX";return false; }
	bool IsReady()const { return false; }
	std::string LastErr()const { return "not built with ONNX"; }
	bool OnGpu()const { return false; }
	bool Generate(const std::vector<uint8_t>&, int, int, int, unsigned, std::vector<uint8_t>&, int&, int&, double*, std::string& errOut) {
		errOut = "not built with ONNX";return false;
	}
	void SetCpuOnly() {}
	bool CpuOnly()const { return true; }
	void Release() {}
};
#endif
static MoebiusEngine g_mbEngine;
static bool g_mbPendingStart = false;
static void ApplyVisionFx(AppConfig& cfg, const ShaderStyle& st) {
	float I = g_styleIntensity;
	ShaderStyle s2 = st;
	s2.outlineStrength *= I;
	s2.bloom *= I;
	s2.saturation = 1.0f + (st.saturation - 1.0f) * I;
	s2.fogDensity *= I;
	s2.toonLevels = st.toonLevels > 0 ? 2.0f + (st.toonLevels - 2.0f) * I : 0;
	s2.rimLighting *= I;
	s2.paletteMix *= I;
	s2.edgeSoftness *= I;
	s2.vignette *= I;
	s2.grain *= I;
	s2.chromatic *= I;
	s2.neonGlow *= I;
	s2.skyGlow *= I;
	s2.rain *= I;
	s2.wind *= I;
	const ShaderStyle& st2 = s2;
	cfg.fx.outlineEnabled = st2.outlineEnabled;
	cfg.fx.outlineThickness = 1.0f + st2.outlineStrength;
	cfg.fx.bloomIntensity = st2.bloom;
	cfg.fx.vibranceAmount = (st2.saturation - 1.0f) * 0.7f;
	cfg.fx.fogEnabled = st2.fogEnabled;
	cfg.fx.fogDistance = 0.35f;
	cfg.fx.fogDensity = 0.15f + st2.fogDensity * 0.7f;
	cfg.fx.vignetteIntensity = st2.vignette;
	cfg.fx.filmGrainAmount = st2.grain;
	cfg.fx.chromaticAberration = st2.chromatic;
	cfg.fx.neonGlowIntensity = st2.neonGlow;
	cfg.fx.skyGlowStrength = st2.skyGlow;
	cfg.fx.contactShadow = (st2.contrast - 1.0f);
	cfg.fx.themeMode = st2.themeMode;
	cfg.fx.tonemapMode = st2.tonemapMode;
	cfg.fx.rainDrops = st2.rain;
	cfg.fx.windDistortion = st2.wind;
	if (st2.pixelEnabled) { cfg.fx.customFx[4] = 0.3f;cfg.fx.customFx[5] = st2.pixelMix; }
	else { cfg.fx.customFx[4] = 0.0f;cfg.fx.customFx[5] = 0.0f; }
	cfg.fx.upscaleSD = false;
}
static void ResetVisionFx(AppConfig& cfg) {
	cfg.fx.outlineEnabled = false;cfg.fx.outlineThickness = 1.5f;
	cfg.fx.bloomIntensity = 0;cfg.fx.vibranceAmount = 0;
	cfg.fx.fogEnabled = false;cfg.fx.vignetteIntensity = 0;cfg.fx.filmGrainAmount = 0;
	cfg.fx.chromaticAberration = 0;cfg.fx.neonGlowIntensity = 0;cfg.fx.skyGlowStrength = 0;
	cfg.fx.contactShadow = 0;cfg.fx.themeMode = 0;cfg.fx.tonemapMode = 0;
	cfg.fx.customFx[4] = 0;cfg.fx.customFx[5] = 0;
	cfg.fx.upscaleSD = false;
}
static std::string m_lastStylePrompt;
static void VisionWorker() {
	if (!g_vision.IsReady()) {
		std::string err;
		g_sdStatus = "Loading vision AI...";
		if (!g_vision.Init(err)) {
			g_visionLastErr = err;
			g_sdStatus = "StudReshader AI failed to load: " + err;
			g_visionRun.store(false);
			g_visionThreadDone.store(true);
			return;
		}
		g_sdEpInfo = "Engine: " + g_vision.EpName();
	}
	g_sdStatus = "StudReshader AI is live | tuning shaders from scene understanding.";
	int lastVer = 0, samCounter = 0;
	try {
		while (g_visionRun.load()) {
			g_lastVisionBeat.store(std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count());
			int ver = g_liveFrameVer.load();
			if (ver == lastVer || ver == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(2));continue; }
			lastVer = ver;
			std::vector<uint8_t> frame;int fw = 0, fh = 0;
			{
				std::lock_guard<std::mutex> lk(g_liveFrameMtx);
				if (!g_liveFrameBytes.empty()) { frame = g_liveFrameBytes;fw = g_liveFW;fh = g_liveFH; }
			}
			if (frame.empty()) { std::this_thread::sleep_for(std::chrono::milliseconds(2));continue; }
			auto t0 = std::chrono::steady_clock::now();
			VisionScene sc;
			VisionStats st;
			std::vector<VisionDetection> dets;
			auto tY = std::chrono::steady_clock::now();
			{
				std::lock_guard<std::mutex> onk(g_onnxMtx);
				if (g_vision.RunYolo(frame, fw, fh, dets))st.yoloMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tY).count();
			}
			for (auto& d : dets) {
				sc.dets.push_back(d);
				if (d.cls == 0)sc.hasPerson = true;
				if (d.cls == 1 || d.cls == 2 || d.cls == 3 || d.cls == 5 || d.cls == 6 || d.cls == 7 || d.cls == 8)sc.hasVehicle = true;
			}
			auto tD = std::chrono::steady_clock::now();
			{
				std::vector<float> depth;int dsz = 0;
				if (g_vision.RunDepth(frame, fw, fh, depth, dsz, sc.skyFrac, sc.groundFrac, sc.avgDepth))
					st.depthMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tD).count();
			}
			sc.seq = ver;
			static ShaderStyle s_baseStyle;
			static bool s_baseValid = false;
			static int s_lastPreset = -2;
			bool compute = false;
			{
				std::lock_guard<std::mutex> lk(g_visionMtx);
				if (g_visionPromptDirty.exchange(false))compute = true;
			}
			int curPreset = g_visionPresetSel.load();
			if (curPreset >= 0 && curPreset < kStylePresetCount) {
				if (!s_baseValid || s_lastPreset != curPreset) {
					s_baseStyle = kStylePresets[curPreset].s;
					s_baseStyle.name = kStylePresets[curPreset].name;
					s_baseValid = true;
					s_lastPreset = curPreset;
				}
			}
			else {
				std::string prompt;
				{ std::lock_guard<std::mutex> lk(g_visionMtx);prompt = g_visionPrompt; }
				if (compute || !s_baseValid || m_lastStylePrompt != prompt) {
					s_baseStyle = g_vision.PromptToStyle(prompt);
					m_lastStylePrompt = prompt;
					s_baseValid = true;
					s_lastPreset = -1;
				}
			}
			static float sSky = 0.0f, sGround = 0.0f;
			sSky += (sc.skyFrac - sSky) * 0.25f;
			sGround += (sc.groundFrac - sGround) * 0.25f;
			ShaderStyle style = s_baseStyle;
			ApplySceneToStyle(style, sc.hasPerson, sc.hasVehicle, sSky, sGround);
			st.clipMs = g_vision.LastClipMs();
			++samCounter;
			if (samCounter >= 25 && sc.hasPerson) {
				samCounter = 0;
				for (auto& d : sc.dets)if (d.cls == 0 && d.conf > 0.4f) {
					auto tS = std::chrono::steady_clock::now();
					std::vector<uint8_t> mask;int mw = 0, mh = 0;
					if (g_vision.RunSam(frame, fw, fh, d.cx, d.cy, d.w, d.h, mask, mw, mh)) {
						st.samMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tS).count();
						{ std::lock_guard<std::mutex> lk(g_maskPubMtx);g_maskPub = std::move(mask);g_maskPubW = mw;g_maskPubH = mh; }
						g_maskPubDirty.store(true);
					}
					break;
				}
			}
			else if (samCounter >= 60 && !sc.hasPerson) {
				samCounter = 0;
				std::vector<uint8_t> z;
				{ std::lock_guard<std::mutex> lk(g_maskPubMtx);g_maskPub.swap(z);g_maskPubW = 0;g_maskPubH = 0; }
				g_maskPubDirty.store(true);
			}
			st.objects = (int)sc.dets.size();
			st.totalMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
			{
				std::lock_guard<std::mutex> lk(g_visionMtx);
				g_visionStyle = style;
				g_visionScene = sc;
				g_visionStats = st;
				{
					std::lock_guard<std::mutex> lk2(g_visUndMtx);
					g_visionUnderstanding = g_vision.Understanding();
				}
				if (st.totalMs > 0) {
					double fps = 1000.0 / st.totalMs;
					g_visionStats.fps = g_visionStats.fps > 0 ? g_visionStats.fps * 0.7 + fps * 0.3 : fps;
				}
			}
		}
	}
	catch (const std::exception& e) {
		g_visionLastErr = std::string("vision crash: ") + e.what();
		g_sdStatus = g_visionLastErr;
	}
	catch (...) {
		g_visionLastErr = "AI crash(unknown)";
		g_sdStatus = g_visionLastErr;
	}
	g_visionThreadDone.store(true);
}
static void VisionStart() {
	if (g_visionRun.load())return;
	if (g_visionThread.joinable()) {
		for (int i = 0;i < 2000 && g_visionThread.joinable() && !g_visionThreadDone.load();++i)Sleep(5);
		if (g_visionThread.joinable())g_visionThread.detach();else g_visionThread.join();
	}
	g_vision.SetModelUse(g_modelYolo, g_modelSam, g_modelDepth, g_modelClip);
	g_visionThreadDone.store(false);
	g_visionRun.store(true);
	g_visionThread = std::thread(VisionWorker);
	g_visionEnabled = true;
	g_sdStatus = "StudReshader AI starting...";
}
static void VisionStop(AppConfig& cfg) {
	if (!g_visionRun.load())return;
	g_visionRun.store(false);
	if (g_visionThread.joinable()) {
		for (int i = 0;i < 2000 && g_visionThread.joinable() && !g_visionThreadDone.load();++i)Sleep(5);
		if (g_visionThread.joinable())g_visionThread.detach();else g_visionThread.join();
	}
	g_visionEnabled = false;
	ResetVisionFx(cfg);
	SaveConfig(cfg);
	g_sdStatus = "StudReshader AI stopped | shaders back to neutral.";
}
static std::string g_aiJsonDisplay;
struct CommShader {
	std::string id, title, creator, desc;
	int64_t uses = 0;
	std::string fxJson;
	std::string styleName;
};
static std::vector<CommShader> g_commShaders;
static std::map<std::string, int64_t> g_commLocalUses;
static void CommLoadLocalUses() {
	g_commLocalUses.clear();
	std::ifstream f{ std::filesystem::path(L"shader_uses.dat") };
	if (!f.is_open())return;
	std::string id;
	int64_t n;
	while (f >> id >> n)g_commLocalUses[id] = n;
}
static void CommSaveLocalUses() {
	std::ofstream f{ std::filesystem::path(L"shader_uses.dat") };
	if (!f.is_open())return;
	for (auto& kv : g_commLocalUses)f << kv.first << " " << kv.second << "\n";
}
static int64_t CommTotalUses(const CommShader& cs) {
	int64_t local = 0;
	auto it = g_commLocalUses.find(cs.id.empty() ? cs.title : cs.id);
	if (it != g_commLocalUses.end())local = it->second;
	return cs.uses + local;
}
static std::string CommCreator(const CommShader& cs) {
	if (cs.creator.empty() || cs.creator == "YourName" || cs.creator == "yourname" || cs.creator == "Your Name")
		return "StudReshader";
	return cs.creator;
}
static bool g_commLoaded = false, g_commLoading = false, g_commError = false;
static std::string g_commErr;
static char g_commSearch[128] = "";
static std::thread g_commThread;
static bool g_commHasShown = false;
static std::string g_commShareText;
static bool g_commShareReady = false;
static const wchar_t* kCommShadersUrl = L"https://raw.githubusercontent.com/STUDWORKS/studworks-shaders/main/shaders.json";
static std::string JsonObjToStr(const sd15::Json& o) {
	std::string out = "{";
	bool first = true;
	for (auto& kv : o.obj) {
		if (!first)out += ",";
		first = false;
		out += "\"" + kv.first + "\":";
		switch (kv.second.t) {
		case sd15::Json::T::Num: { char b[48];sprintf_s(b, "%.6g", kv.second.n);out += b;break; }
		case sd15::Json::T::Str: out += "\"" + kv.second.s + "\"";break;
		case sd15::Json::T::Bool: out += kv.second.b ? "true" : "false";break;
		case sd15::Json::T::Arr: {
			out += "[";
			bool f2 = true;
			for (auto& v : kv.second.arr) { if (!f2)out += ",";f2 = false;char b[48];sprintf_s(b, "%.6g", v.n);out += b; }
			out += "]";
			break;
		}
		default: out += "null";
		}
	}
	out += "}";
	return out;
}
static char g_pubTitle[96] = "";
static char g_pubCreator[48] = "StudReshader";
static char g_pubDesc[300] = "";
static std::thread g_pubThread;
static char g_pubToken[64] = "";
static char g_pubRepo[64] = "STUDWORKS/studworks-shaders";
static bool g_pubBusy = false;
static std::string g_pubMsg;
static std::string g_pubSha;
static void CommLoadPubAuth() {
	std::ifstream f{ std::filesystem::path(L"ghauth.dat") };
	if (!f.is_open())return;
	std::string line;
	std::getline(f, line);if (line.size() < 64)memcpy(g_pubToken, line.c_str(), line.size());
	std::getline(f, line);if (line.size() < 64)memcpy(g_pubRepo, line.c_str(), line.size());
}
static void CommSavePubAuth() {
	std::ofstream f{ std::filesystem::path(L"ghauth.dat") };
	if (!f.is_open())return;
	f << g_pubToken << "\n" << g_pubRepo << "\n";
}
static std::string UrlEncode(const std::string& s) {
	static const char* hex = "0123456789ABCDEF";
	std::string out;
	for (unsigned char ch : s) {
		if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~')out += ch;
		else { out += '%';out += hex[ch >> 4];out += hex[ch & 15]; }
	}
	return out;
}
static bool GitHubPutContents(const std::string& owner, const std::string& repo, const std::string& token,
	const std::string& content, const std::string& sha, std::string& errOut) {
	std::string url = "https://api.github.com/repos/" + owner + "/" + repo + "/contents/shaders.json";
	URL_COMPONENTS uc{};uc.dwStructSize = sizeof(uc);
	std::vector<wchar_t> host(256, 0), path(4096, 0);
	uc.lpszHostName = host.data();uc.dwHostNameLength = (DWORD)host.size();
	uc.lpszUrlPath = path.data();uc.dwUrlPathLength = (DWORD)path.size();
	std::wstring wurl(url.begin(), url.end());
	if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &uc)) { errOut = "bad url";return false; }
	HINTERNET ses = WinHttpOpen(L"Mozilla/5.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!ses) { errOut = "network";return false; }
	HINTERNET con = WinHttpConnect(ses, uc.lpszHostName, uc.nPort, 0);
	if (!con) { WinHttpCloseHandle(ses);errOut = "connect";return false; }
	HINTERNET req = WinHttpOpenRequest(con, L"PUT", uc.lpszUrlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);errOut = "request";return false; }
	std::string auth = "Authorization: Bearer " + token;
	std::wstring wauth(auth.begin(), auth.end());
	std::wstring ctype = L"Content-Type: application/json\r\n";
	std::wstring headers = wauth + L"\r\n" + ctype;
	std::string body = content;
	BOOL ok = WinHttpSendRequest(req, headers.c_str(), -1L, (LPVOID)body.data(), (DWORD)body.size(), (DWORD)body.size(), 0);
	if (ok)ok = WinHttpReceiveResponse(req, nullptr);
	if (!ok) { WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);errOut = "send failed";return false; }
	DWORD status = 0, sz = sizeof(status);
	WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
	WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
	if (status == 200 || status == 201)return true;
	errOut = "GitHub returned HTTP " + std::to_string(status);
	return false;
}
static bool GitHubFetchMeta(const std::string& owner, const std::string& repo, const std::string& token,
	std::string& jsonOut, std::string& shaOut, std::string& errOut) {
	std::string url = "https://api.github.com/repos/" + owner + "/" + repo + "/contents/shaders.json";
	URL_COMPONENTS uc{};uc.dwStructSize = sizeof(uc);
	std::vector<wchar_t> host(256, 0), path(4096, 0);
	uc.lpszHostName = host.data();uc.dwHostNameLength = (DWORD)host.size();
	uc.lpszUrlPath = path.data();uc.dwUrlPathLength = (DWORD)path.size();
	std::wstring wurl(url.begin(), url.end());
	if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &uc)) { errOut = "bad url";return false; }
	HINTERNET ses = WinHttpOpen(L"Mozilla/5.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!ses)return false;
	HINTERNET con = WinHttpConnect(ses, uc.lpszHostName, uc.nPort, 0);
	if (!con) { WinHttpCloseHandle(ses);return false; }
	HINTERNET req = WinHttpOpenRequest(con, L"GET", uc.lpszUrlPath, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);return false; }
	std::string auth = "Authorization: Bearer " + token;
	std::wstring wauth(auth.begin(), auth.end());
	std::wstring accept = L"Accept: application/vnd.github+json\r\n";
	std::wstring headers = wauth + L"\r\n" + accept;
	BOOL ok = WinHttpSendRequest(req, headers.c_str(), -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if (ok)ok = WinHttpReceiveResponse(req, nullptr);
	if (!ok) { WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);errOut = "send failed";return false; }
	DWORD status = 0, sz = sizeof(status);
	WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
	std::string body;
	if (status == 200) {
		DWORD avail = 0;
		while (WinHttpQueryDataAvailable(req, &avail) && avail > 0) {
			std::vector<char> buf(avail);
			DWORD got = 0;
			if (!WinHttpReadData(req, buf.data(), avail, &got) || got == 0)break;
			body.append(buf.data(), got);
		}
	}
	WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
	if (status != 200) { errOut = "HTTP " + std::to_string(status);return false; }
	size_t p = body.find("\"sha\":");
	if (p != std::string::npos) {
		p += 6;
		size_t q = body.find('"', p);
		shaOut = body.substr(p, q - p);
	}
	p = body.find("\"content\":");
	if (p != std::string::npos) {
		p += 10;
		size_t q = body.find('"', p);
		std::string b64 = body.substr(p, q - p);
		std::string clean;
		for (char ch : b64)if (ch != '\n' && ch != '\r')clean += ch;
		static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		int val = 0, bits = -8;
		for (char ch : clean) {
			if (ch == '=')break;
			const char* f = strchr(tbl, ch);
			if (!f)continue;
			val = (val << 6) + (int)(f - tbl);
			bits += 6;
			if (bits >= 0) {
				jsonOut += (char)((val >> bits) & 0xFF);
				bits -= 8;
			}
		}
	}
	return !jsonOut.empty();
}
static void CommPublish(AppConfig& cfg) {
	if (g_pubBusy)return;
	g_pubBusy = true;
	g_pubMsg = "Validating and publishing...";
	if (g_pubThread.joinable())g_pubThread.detach();
	g_pubThread = std::thread([=] {
		CommSavePubAuth();
		std::string title(g_pubTitle), creator(g_pubCreator), desc(g_pubDesc), token(g_pubToken), repo(g_pubRepo);
		std::string err;
		if (title.empty() || creator.empty()) { g_pubMsg = "Title and creator are required.";g_pubBusy = false;return; }
		size_t slash = repo.find('/');
		if (slash == std::string::npos) { g_pubMsg = "Repo must be owner/repo";g_pubBusy = false;return; }
		std::string owner = repo.substr(0, slash), name = repo.substr(slash + 1);
		std::string curJson, sha;
		if (!GitHubFetchMeta(owner, name, token, curJson, sha, err)) {
			g_pubMsg = "Could not read the repo: " + err + "(create the repo and add shaders.json first)";
			g_pubBusy = false;
			return;
		}
		std::string newEntry = "\n{\n \"title\": \"" + title + "\",\n \"creator\": \"" + creator + "\",\n \"description\": \"" + desc + "\",\n \"uses\": 0,\n \"fx\": " + g_aiJsonDisplay + "\n}";
		std::string outJson = curJson;
		size_t close = outJson.rfind(']');
		if (close == std::string::npos) { g_pubMsg = "shaders.json malformed";g_pubBusy = false;return; }
		std::string before = outJson.substr(0, close);
		bool needsComma = before.find('{') != std::string::npos;
		outJson = before + (needsComma ? "," : "") + newEntry + "\n]" + outJson.substr(close + 1);
		static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string b64;
		int val = 0, bits = 0;
		for (char ch : outJson) {
			val = (val << 8) | (unsigned char)ch;
			bits += 8;
			while (bits >= 6) { b64 += tbl[(val >> (bits - 6)) & 63];bits -= 6; }
		}
		if (bits > 0)b64 += tbl[(val << (6 - bits)) & 63];
		while (b64.size() % 4)b64 += '=';
		std::string putBody = "{\"message\":\"Add shader: " + title + "\",\"content\":\"" + b64 + "\"";
		if (!sha.empty())putBody += ",\"sha\":\"" + sha + "\"";
		putBody += "}";
		if (GitHubPutContents(owner, name, token, putBody, sha, err)) {
			g_pubMsg = "Published! The shader is now live in the community list.";
			g_sdStatus = "Shader published to the community repo.";
		}
		else {
			g_pubMsg = "Publish failed: " + err;
		}
		g_pubBusy = false;
		});
}
static void CommFetch() {
	if (g_commLoading)return;
	CommLoadLocalUses();
	g_commLoading = true;
	g_commError = false;
	g_commErr.clear();
	if (g_commThread.joinable())g_commThread.detach();
	g_commThread = std::thread([] {
		std::wstring tmp = L"studworks_shaders_cache.json";
		std::string err;
		bool ok = DownloadUrlToFile(kCommShadersUrl, tmp, nullptr, &err);
		if (!ok) { g_commErr = err;g_commError = true;g_commLoading = false;return; }
		std::ifstream f{ std::filesystem::path(tmp) };
		std::stringstream ss;ss << f.rdbuf();
		f.close();
		std::error_code ec;std::filesystem::remove(tmp, ec);
		std::string json = ss.str();
		std::vector<CommShader> out;
		try {
			sd15::Json doc = sd15::JsonParser(json).parse();
			const sd15::Json* arr = (doc.t == sd15::Json::T::Obj) ? doc.find("shaders") : nullptr;
			if (!arr || arr->t != sd15::Json::T::Arr) { g_commErr = "shaders.json has no valid 'shaders' array";g_commError = true;g_commLoading = false;return; }
			for (auto& e : arr->arr) {
				if (e.t != sd15::Json::T::Obj)continue;
				CommShader cs;
				const sd15::Json* t = e.find("title");
				if (!t || t->t != sd15::Json::T::Str || t->s.empty() || t->s.size() > 80)continue;
				cs.title = t->s;
				const sd15::Json* cr = e.find("creator");
				if (!cr || cr->t != sd15::Json::T::Str || cr->s.empty() || cr->s.size() > 40)continue;
				cs.creator = cr->s;
				const sd15::Json* d = e.find("description");
				if (d && d->t == sd15::Json::T::Str)cs.desc = d->s.substr(0, 300);
				const sd15::Json* u = e.find("uses");
				if (u && u->t == sd15::Json::T::Num)cs.uses = (int64_t)u->n;
				if (cs.uses < 0)cs.uses = 0;
				const sd15::Json* fx = e.find("fx");
				if (!fx || fx->t != sd15::Json::T::Obj)continue;
				sd15::Json clean;
				clean.t = sd15::Json::T::Obj;
				for (auto& kv : fx->obj) {
					if (kv.second.t == sd15::Json::T::Num || kv.second.t == sd15::Json::T::Arr) {
						clean.obj.push_back(kv);
					}
				}
				if (clean.obj.empty())continue;
				cs.fxJson = JsonObjToStr(clean);
				if (cs.fxJson.size() > 8192)continue;
				const sd15::Json* sn = e.find("style");
				if (sn && sn->t == sd15::Json::T::Str)cs.styleName = sn->s.substr(0, 40);
				const sd15::Json* id = e.find("id");
				if (id && id->t == sd15::Json::T::Str)cs.id = id->s.substr(0, 60);
				out.push_back(std::move(cs));
			}
		}
		catch (...) {
			g_commErr = "failed to parse shaders.json";
			g_commError = true;
			g_commLoading = false;
			return;
		}
		std::sort(out.begin(), out.end(), [](const CommShader& a, const CommShader& b) {return a.uses > b.uses;});
		g_commShaders = std::move(out);
		g_commLoaded = true;
		g_commLoading = false;
		});
}
static void CommUseShader(AppConfig& cfg, const CommShader& cs) {
	std::string key = cs.id.empty() ? cs.title : cs.id;
	g_commLocalUses[key] = g_commLocalUses[key] + 1;
	CommSaveLocalUses();
	EffectSettings fx = cfg.fx;
	if (!cs.fxJson.empty() && JsonToEffectSettings(cs.fxJson, fx)) {
		cfg.fx = fx;
	}
	if (!cs.styleName.empty()) {
		for (int i = 0;i < kStylePresetCount;++i) {
			if (strncmp(kStylePresets[i].name, cs.styleName.c_str(), 40) == 0) {
				g_activeStyle = kStylePresets[i].s;
				g_activeStyle.name = kStylePresets[i].name;
				g_gpuDrawMode = true;
				g_drawPreset = i;
				ApplyVisionFx(cfg, g_activeStyle);
				break;
			}
		}
	}
	SaveConfig(cfg);
	g_sdStatus = "Applied community shader: " + cs.title + "(made by " + CommCreator(cs) + ")";
	g_aiJsonDisplay = EffectSettingsToJson(cfg.fx);
}
static void ApplyGpuDrawing(AppConfig& cfg, int presetIdx) {
	if (presetIdx < 0 || presetIdx >= kStylePresetCount)presetIdx = 0;
	g_activeStyle = kStylePresets[presetIdx].s;
	g_activeStyle.name = kStylePresets[presetIdx].name;
	ApplyVisionFx(cfg, g_activeStyle);
	cfg.fx.upscaleSD = false;
	g_drawPreset = presetIdx;
	g_gpuDrawMode = true;
	if (g_visionEnabled)VisionStop(cfg);
	g_sdStatus = "Drawing mode: " + std::string(kStylePresets[presetIdx].name) + "(GPU shaders,no AI needed)";
}
static void StopGpuDrawing(AppConfig& cfg) {
	g_gpuDrawMode = false;
	ResetVisionFx(cfg);
	g_sdStatus = "Drawing mode off | shaders back to neutral.";
}
static DepthEngine* g_depthEnginePtr = nullptr;
static int g_liveIntervalMs = 0;
static std::chrono::steady_clock::time_point g_lastGenT{};
static bool g_wlPendingStart = false;
static bool g_dsPendingStart = false;
static int g_livePresetSel = 0;
static bool g_autoDropped = false;
static bool g_autoStepped = false;
static bool g_anchorOn = false;
static bool g_anchorReady = false;
static std::vector<uint8_t> g_anchorPatch;
static int g_anchorPW = 0, g_anchorPH = 0;
static float g_anchorX = 0.0f, g_anchorY = 0.0f;
static float g_anchorScale = 1.0f;
static std::vector<uint8_t> g_prevCapY;
static int g_prevCapW = 0, g_prevCapH = 0;
static std::atomic<bool> g_objReady{ false };
static std::atomic<bool> g_objDirty{ false };
static std::mutex g_objMtx;
static std::vector<uint8_t> g_objPatch;
static int g_objPW = 0, g_objPH = 0;
static float g_objCX = 0.0f, g_objCY = 0.0f;
static float g_objCW = 0.0f, g_objCH = 0.0f;
static float g_objScale = 1.0f;
static ID3D11ShaderResourceView* g_objSrv = nullptr;
static std::vector<uint8_t> g_objPrevY;
static std::vector<uint8_t> g_objRefY;
static bool g_objRefSet = false;
static float g_objRefCX = 0.0f, g_objRefCY = 0.0f;
static float g_objRefCW = 0.0f, g_objRefCH = 0.0f;
static float g_objRefScale = 1.0f;
static bool g_radarOn = false;
static std::atomic<bool> g_radarRun{ false };
static std::thread g_radarThread;
static std::atomic<bool> g_radarThreadDone{ true };
static std::mutex g_radarMtx;
static std::vector<VisionDetection> g_radarDets;
static int g_radarFW = 0, g_radarFH = 0;
static std::string g_radarErr;
static float g_radarConf = 0.20f;
static bool g_radarPlayersOnly = true;
static bool g_radarZoom = true;
static std::string g_gameName;
static std::mutex g_radarFrameMtx;
static std::vector<uint8_t> g_radarFrameBytes;
static int g_radarFrameW = 0, g_radarFrameH = 0;
static std::atomic<int> g_radarFrameVer{ 0 };
static std::atomic<int> g_radarYoloOk{ 0 };
static std::atomic<int> g_radarRawDets{ 0 };
static std::atomic<double> g_radarLastFrameT{ 0.0 };
static std::atomic<float> g_radarFocusX{ 0.5f };
static std::atomic<float> g_radarFocusY{ 0.5f };
static ComPtr<ID3D11Texture2D> g_radarStaging;
static UINT g_radarStagingW = 0, g_radarStagingH = 0;
static RszSlot g_radarSlot;
static bool g_coachOn = false;
static float g_coachFont = 30.0f;
static float g_coachOpacity = 0.92f;
static float g_coachCol[3] = { 0.15f,0.85f,0.4f };
static int g_kbLayout = 0;
static bool g_kbAzerty = false;
static std::string g_coachAiText;
static std::mutex g_coachTextMtx;
static std::atomic<bool> g_coachAiBusy{ false };
static int g_coachTargetId = -1;
static double g_coachLastAskT = -10.0;
static std::mutex g_cfgStrMtx;
static std::atomic<bool> g_ollamaPulling{ false };
static void SetStatus(const std::string& s);
static std::string GetStatus();
static void DetectKeyboardLayout() {
	if (g_kbLayout == 1) { g_kbAzerty = false;return; }
	if (g_kbLayout == 2) { g_kbAzerty = true;return; }
	g_kbAzerty = false;
	HKL kl = GetKeyboardLayout(0);
	LANGID lang = (LANGID)((uintptr_t)kl & 0xFFFF);
	switch (lang) {
	case 0x040C: case 0x080C: case 0x0C0C: case 0x100C: case 0x140C: case 0x180C:
	case 0x0813:
		g_kbAzerty = true;
		break;
	default:
		g_kbAzerty = false;
	}
}
static const char* KbKey(const char* qwertyKey) {
	DetectKeyboardLayout();
	if (!g_kbAzerty)return qwertyKey;
	if (strcmp(qwertyKey, "W") == 0)return "Z";
	if (strcmp(qwertyKey, "A") == 0)return "Q";
	return qwertyKey;
}
static std::wstring g_objDbgDir;
static std::string g_objDbgPath;
static std::string WtoA(const std::wstring& w) {
	std::string out;
	if (w.empty())return out;
	std::vector<char> tmp(w.size() * 4 + 4);
	int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), tmp.data(), (int)tmp.size(), nullptr, nullptr);
	if (n > 0)out.assign(tmp.data(), (size_t)n);
	return out;
}
static int g_anchorBaseDepth = 0;
static bool PromptHasObject(const std::string& p) {
	std::string l = p;
	std::transform(l.begin(), l.end(), l.begin(), [](unsigned char c) {return (char)std::tolower(c);});
	static const char* words[] = { "bear","dog","cat","dragon","robot","zombie","monster","wolf","tiger","lion","horse","knight","soldier","alien","dinosaur","shark","whale","spider","ghost","ninja","samurai","person","character","creature","animal" };
	for (auto w : words)if (l.find(w) != std::string::npos)return true;
	return false;
}
static void CapLuma(const std::vector<uint8_t>& bgra, int w, int h, std::vector<uint8_t>& y, int& ow, int& oh) {
	ow = 64;
	oh = std::max(16, h * 64 / std::max(1, w));
	y.assign((size_t)ow * oh, 0);
	for (int yy = 0;yy < oh;++yy) {
		int sy = yy * h / oh;
		for (int xx = 0;xx < ow;++xx) {
			int sx = xx * w / ow;
			const uint8_t* p = &bgra[((size_t)sy * w + sx) * 4];
			y[(size_t)yy * ow + xx] = (uint8_t)(((unsigned)p[2] * 77 + (unsigned)p[1] * 150 + (unsigned)p[0] * 29) >> 8);
		}
	}
}
static void EstimateMotion(const std::vector<uint8_t>& cur, const std::vector<uint8_t>& prev, int w, int h, float& dx, float& dy) {
	dx = 0.0f;dy = 0.0f;
	if (cur.size() != prev.size() || w < 16 || h < 8)return;
	long long best = 1LL << 60;
	int bx = 0, by = 0;
	for (int oy = -10;oy <= 10;++oy) {
		for (int ox = -10;ox <= 10;++ox) {
			long long s = 0;
			for (int yy = 3;yy < h - 3;yy += 2) {
				const uint8_t* cr = &cur[(size_t)yy * w];
				for (int xx = 3;xx < w - 3;xx += 2) {
					int cx = xx + ox, cy = yy + oy;
					if (cx < 0 || cy < 0 || cx >= w || cy >= h)continue;
					int d = (int)cr[xx] - (int)prev[(size_t)cy * w + cx];
					s += (long long)d * d;
				}
			}
			if (s < best) { best = s;bx = ox;by = oy; }
		}
	}
	dx = (float)bx;
	dy = (float)by;
}
static void AnchorInitFromDiff(const std::vector<uint8_t>& cap, int cw, int ch, const std::vector<uint8_t>& out, int ow, int oh) {
	g_anchorReady = false;
	g_anchorPatch.clear();
	g_anchorPW = g_anchorPH = 0;
	std::vector<uint8_t> cy, oy;
	int cW = 0, cH = 0, oW = 0, oH = 0;
	CapLuma(cap, cw, ch, cy, cW, cH);
	CapLuma(out, ow, oh, oy, oW, oH);
	int W = std::min(cW, oW), H = std::min(cH, oH);
	std::vector<int> diff((size_t)W * H, 0);
	int bx0 = W, by0 = H, bx1 = -1, by1 = -1;
	for (int yy = 0;yy < H;++yy) {
		for (int xx = 0;xx < W;++xx) {
			int d = abs((int)oy[(size_t)yy * oW + xx] - (int)cy[(size_t)yy * cW + xx]);
			diff[(size_t)yy * W + xx] = d;
			if (d > 26) {
				if (xx < bx0)bx0 = xx;
				if (xx > bx1)bx1 = xx;
				if (yy < by0)by0 = yy;
				if (yy > by1)by1 = yy;
			}
		}
	}
	if (bx1 <= bx0 || by1 <= by0) {
		std::vector<int> vals(diff.begin(), diff.end());
		std::sort(vals.begin(), vals.end());
		if (vals.empty() || vals.back() < 12)return;
		int thr = vals[(size_t)((double)vals.size() * 0.86)];
		if (thr < 12)thr = 12;
		bx0 = W;by0 = H;bx1 = -1;by1 = -1;
		for (int yy = 0;yy < H;++yy)
			for (int xx = 0;xx < W;++xx)
				if (diff[(size_t)yy * W + xx] >= thr) {
					if (xx < bx0)bx0 = xx;
					if (xx > bx1)bx1 = xx;
					if (yy < by0)by0 = yy;
					if (yy > by1)by1 = yy;
				}
		if (bx1 <= bx0 || by1 <= by0)return;
	}
	int area = (bx1 - bx0 + 1) * (by1 - by0 + 1);
	if (area <(W * H) / 600 || area >(W * H) * 3 / 4)return;
	int padX = (bx1 - bx0 + 1) / 4 + 2;
	int padY = (by1 - by0 + 1) / 4 + 2;
	bx0 = std::max(0, bx0 - padX);bx1 = std::min(W - 1, bx1 + padX);
	by0 = std::max(0, by0 - padY);by1 = std::min(H - 1, by1 + padY);
	int pw = bx1 - bx0 + 1, ph = by1 - by0 + 1;
	float oxf = (float)ow / (float)oW, oyf = (float)oh / (float)oH;
	g_anchorPW = (int)(pw * oxf);
	g_anchorPH = (int)(ph * oyf);
	g_anchorX = (float)(bx0 + bx1) * 0.5f * oxf;
	g_anchorY = (float)(by0 + by1) * 0.5f * oyf;
	g_anchorScale = 1.0f;
	g_anchorPatch.assign((size_t)g_anchorPW * g_anchorPH * 4, 0);
	for (int yy = 0;yy < g_anchorPH;++yy) {
		int sy = by0 + (int)((float)yy / oyf);
		for (int xx = 0;xx < g_anchorPW;++xx) {
			int sx = bx0 + (int)((float)xx / oxf);
			int dy_ = diff[(size_t)sy * W + sx];
			float a = dy_ > 26 ? 1.0f : (dy_ > 12 ? (float)(dy_ - 12) / 14.0f : 0.0f);
			const uint8_t* p = &out[((size_t)(by0 + yy) * ow + (bx0 + xx)) * 4];
			uint8_t* d = &g_anchorPatch[((size_t)yy * g_anchorPW + xx) * 4];
			d[0] = p[0];d[1] = p[1];d[2] = p[2];
			d[3] = (uint8_t)(a * 255.0f);
		}
	}
	g_anchorReady = true;
}
static void AnchorComposite(std::vector<uint8_t>& out, int ow, int oh) {
	if (!g_anchorReady || g_anchorPatch.empty() || g_anchorPW <= 0 || g_anchorPH <= 0)return;
	int pw = (int)(g_anchorPW * g_anchorScale);
	int ph = (int)(g_anchorPH * g_anchorScale);
	if (pw < 8 || ph < 8 || pw > ow * 2 || ph > oh * 2)return;
	int x0 = (int)(g_anchorX - pw * 0.5f);
	int y0 = (int)(g_anchorY - ph * 0.5f);
	if (x0 + pw < 0 || y0 + ph < 0 || x0 >= ow || y0 >= oh)return;
	for (int yy = 0;yy < ph;++yy) {
		int oy = y0 + yy;
		if (oy < 0 || oy >= oh)continue;
		int sy = yy * g_anchorPH / ph;
		for (int xx = 0;xx < pw;++xx) {
			int ox = x0 + xx;
			if (ox < 0 || ox >= ow)continue;
			int sx = xx * g_anchorPW / pw;
			const uint8_t* p = &g_anchorPatch[((size_t)sy * g_anchorPW + sx) * 4];
			uint8_t a = p[3];
			if (a == 0)continue;
			uint8_t* d = &out[((size_t)oy * ow + ox) * 4];
			float af = a / 255.0f;
			d[0] = (uint8_t)(p[0] * af + d[0] * (1.0f - af));
			d[1] = (uint8_t)(p[1] * af + d[1] * (1.0f - af));
			d[2] = (uint8_t)(p[2] * af + d[2] * (1.0f - af));
		}
	}
}
#if defined(_MSC_VER)
static bool SehCall(bool(*fn)(void*), void* ctx, std::string& err) {
	__try { return fn(ctx); }
	__except (EXCEPTION_EXECUTE_HANDLER) { err = "GPU fault";return false; }
}
#else
static bool SehCall(bool(*fn)(void*), void* ctx, std::string& err) { return fn(ctx); }
#endif
struct FastGenCtx { SdPipeline* p;const std::string* prompt;int steps;int seed;int W;int H;const std::vector<uint8_t>* frame;int fw;int fh;float str;std::vector<uint8_t>* out;const std::atomic<bool>* cancel;SdStats* st;std::string* err; };
static bool SehFastGen(void* v) { FastGenCtx* c = (FastGenCtx*)v;return c->p->GenerateFast(*c->prompt, c->steps, c->seed, c->W, c->H, *c->frame, c->fw, c->fh, c->str, *c->out, nullptr, c->cancel, c->st, *c->err); }
struct FullGenCtx { SdPipeline* p;const std::string* prompt;const std::string* neg;int steps;float cfg;int seed;int W;int H;const std::vector<uint8_t>* frame;int fw;int fh;float str;std::vector<uint8_t>* out;const std::atomic<bool>* cancel;SdStats* st;std::string* err; };
static bool SehFullGen(void* v) { FullGenCtx* c = (FullGenCtx*)v;return c->p->Generate(*c->prompt, *c->neg, c->steps, c->cfg, c->seed, c->W, c->H, c->frame, c->fw, c->fh, c->str, *c->out, nullptr, c->cancel, &c->st->unetMs, *c->err); }
struct MbGenCtx { MoebiusEngine* e;const std::vector<uint8_t>* frame;int fw;int fh;int steps;unsigned seed;std::vector<uint8_t>* out;int* ow;int* oh;double* ms;std::string* err;const std::atomic<bool>* cancel; };
static bool SehMbGen(void* v) { MbGenCtx* c = (MbGenCtx*)v;return c->e->Generate(*c->frame, c->fw, c->fh, c->steps, c->seed, *c->out, *c->ow, *c->oh, c->ms, *c->err, c->cancel); }
static bool ExtractObjFromFlat(const std::vector<uint8_t>&, int, int, const std::vector<uint8_t>&, int, int,
	std::vector<uint8_t>&, int&, int&, float&, float&);
static void SaveObjDebug(const std::vector<uint8_t>&, int, int, int);
static void ObjDbgLog(const std::string&);
static bool LooksLikeNoise(const std::vector<uint8_t>& bgra, int w, int h) {
	if (bgra.empty() || w < 8 || h < 8)return true;
	size_t tot = 0, edges = 0;
	for (int y = 0;y < h;++y) {
		const uint8_t* r0 = &bgra[((size_t)y * w) * 4];
		const uint8_t* r1 = &bgra[((size_t)y * w + 1) * 4];
		for (int x = 0;x + 1 < w;x += 2) {
			int d = std::abs((int)r0[x * 4] - (int)r1[x * 4]) + std::abs((int)r0[x * 4 + 1] - (int)r1[x * 4 + 1]) + std::abs((int)r0[x * 4 + 2] - (int)r1[x * 4 + 2]);
			if (d > 60)++edges;
			++tot;
		}
	}
	return tot > 0 && (double)edges / (double)tot > 0.35;
}
static void LiveWorker() {
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	if (g_liveModel == 2) {
		g_sdPipeline.Release();
		if (!g_mbEngine.IsReady()) {
			std::string me;
			g_sdStatus = "Loading Moebius engine...";
			if (!g_mbEngine.Init(me)) {
				g_liveLastErr = me;
				g_sdStatus = "Moebius load failed: " + me;
				g_liveRun.store(false);
				g_liveThreadDone.store(true);
				return;
			}
		}
		if (g_mbEngine.OnGpu()) {
			std::vector<uint8_t> tw(64 * 36 * 4, 128);
			std::vector<uint8_t> wo;
			int ww = 0, wh = 0;
			std::string we;
			double wms = 0;
			MbGenCtx wctx{ &g_mbEngine,&tw,64,36,1,7,&wo,&ww,&wh,&wms,&we,nullptr };
			std::string weh;
			bool wok = SehCall(SehMbGen, &wctx, weh);
			if (!wok && weh.find("GPU fault") != std::string::npos) {
				g_mbEngine.Release();
				g_mbEngine.SetCpuOnly();
				g_liveGpuOnly = false;
				std::string me;
				if (g_mbEngine.Init(me))g_sdStatus = "Moebius | CPU | GPU fault fallback";
			}
		}
	}
	else {
		g_mbEngine.Release();
		if (!g_sdPipeline.IsReady()) {
			std::string err;
			g_sdStatus = "Loading AI engine...";
			g_liveLastErr.clear();
			if (!g_sdPipeline.Init(err)) {
				g_liveLastErr = g_sdPipeline.LastErr();
				g_sdStatus = "Could not load the model: " + g_liveLastErr;
				g_liveRun.store(false);
				g_liveThreadDone.store(true);
				return;
			}
			g_sdEpInfo = g_sdPipeline.EpName();
		}
		if (g_liveModel == 3 && !SdFilesReady()) {
			g_liveLastErr = "SD 1.5 files missing.";
			g_sdStatus = g_liveLastErr;
			g_liveRun.store(false);
			g_liveThreadDone.store(true);
			return;
		}
		if (!g_sdPipeline.HasFast()) {
			g_liveLastErr = "Fast engine not installed(download the model to unlock live mode).";
			g_sdStatus = g_liveLastErr;
			g_liveRun.store(false);
			g_liveThreadDone.store(true);
			return;
		}
		if (g_liveModel == 0 && !g_sdPipeline.FastOnGpu()) {
			if (g_sdPipeline.HasDs() && g_sdPipeline.DsOnGpu()) {
				g_liveModel = 1;
				g_sdStatus = "DreamShaper V7 engine | GPU";
			}
			else if (g_sdPipeline.HasDs()) {
				g_liveModel = 1;
				g_sdStatus = "DreamShaper V7 engine | CPU";
			}
			else {
				g_liveModel = 1;
				g_dsPendingStart = true;
				if (!g_sdProv.Running()) {
					g_sdProv.Reset();
					g_sdProv.Start(kDsFiles, kDsFileCount);
				}
				g_sdStatus = "Downloading DreamShaper V7(~3.4 GB)";
				g_sdCancel.store(true);
				g_liveRun.store(false);
				g_liveThreadDone.store(true);
				return;
			}
		}
		g_sdPipeline.SetFastModel(g_liveModel);
		if (g_liveModel == 1 && !g_sdPipeline.HasDs()) {
			g_liveLastErr = "DreamShaper V7 not installed | switch back to Fast model or download it.";
			g_sdStatus = g_liveLastErr;
			g_liveRun.store(false);
			g_liveThreadDone.store(true);
			return;
		}
		try { g_sdPipeline.Warmup(); }
		catch (...) { g_sdPipeline.Warmup(); }
	}
	bool isCpu = (g_liveModel == 2) ? !g_mbEngine.OnGpu() : (g_liveModel == 3) ? !g_sdPipeline.MainOnGpu() : !g_sdPipeline.FastOnGpu();
	if (g_liveGpuOnly && isCpu) {
		g_liveLastErr = "GPU not available | engine requires GPU";
		g_sdStatus = "GPU not available | engine requires GPU | check models\\gpu_error.txt";
		if (g_liveModel == 2) {
			std::ofstream gf("models\\gpu_error.txt", std::ios::trunc);
			if (gf)gf << "Moebius\n" << g_mbEngine.LastErr();
		}
		else {
			std::ofstream gf("models\\gpu_error.txt", std::ios::trunc);
			if (gf)gf << g_sdPipeline.EpName() << "\n" << g_sdPipeline.DmlErr();
		}
		g_liveRun.store(false);
		g_liveThreadDone.store(true);
		return;
	}
	if (isCpu) {
		g_sdStatus = "Engine: " + LiveEngineName();
		{
			std::ofstream gf("models\\gpu_error.txt", std::ios::trunc);
			if (gf)gf << g_sdPipeline.EpName() << "\n" << g_sdPipeline.DmlErr();
		}
		std::string dmlE = g_sdPipeline.DmlErr();
		if (!dmlE.empty()) {
			if (dmlE.size() > 180)dmlE.resize(180);
			g_sdStatus += " | GPU error: " + dmlE;
		}
	}
	else {
		g_sdStatus = "Live mode | engine: " + LiveEngineName();
	}
	int lastVer = 0;
	bool upSkipNote = false;
	while (g_liveRun.load()) {
		g_lastWorkerBeat.store(std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count());
		auto tFrame0 = std::chrono::steady_clock::now();
		int ver = g_liveFrameVer.load();
		if (ver == lastVer || ver == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}
		if (isCpu) {
			if (g_liveFps.load() > 3.0) { std::this_thread::sleep_for(std::chrono::milliseconds(40));continue; }
		}
		{
			if (g_liveIntervalMs > 0 && g_liveModel != 4) {
				auto nowT = std::chrono::steady_clock::now();
				if (g_lastGenT.time_since_epoch().count()) {
					int el = (int)std::chrono::duration_cast<std::chrono::milliseconds>(nowT - g_lastGenT).count();
					if (el < g_liveIntervalMs) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
						continue;
					}
				}
				g_lastGenT = nowT;
			}
		}
		lastVer = ver;
		g_sdPipeline.SetFastModel(g_liveModel);
		if (g_liveModel == 1 && !g_sdPipeline.HasDs()) {
			g_liveLastErr = "DreamShaper V7 not installed | switch back to Fast model.";
			g_sdStatus = g_liveLastErr;
			g_liveRun.store(false);
			break;
		}
		std::vector<uint8_t> frame;
		int fw = 0, fh = 0;
		{
			std::lock_guard<std::mutex> lk(g_liveFrameMtx);
			if (!g_liveFrameBytes.empty()) {
				frame = g_liveFrameBytes;
				fw = g_liveFW;
				fh = g_liveFH;
				g_livePubBusy.store(false);
			}
		}
		if (frame.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}
		g_liveStats.captureMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tFrame0).count();
		if (g_liveShowRaw) {
			std::lock_guard<std::mutex> lk(g_liveOutMtx);
			g_liveOuts[g_liveOutSlot] = std::move(frame);
			g_liveOutSlot = (g_liveOutSlot + 1) % 3;
			g_liveOutW = fw;g_liveOutH = fh;
			g_liveOutReady = true;
			g_liveOutVer.store(ver);
			continue;
		}
		{
			std::vector<uint8_t> lum;
			int lw = 0, lh = 0;
			CapLuma(frame, fw, fh, lum, lw, lh);
			double ls = 0.0, ls2 = 0.0;
			for (size_t li = 0;li < lum.size();++li) { ls += lum[li];ls2 += (double)lum[li] * lum[li]; }
			double lmean = ls / (double)lum.size();
			double lstd = std::sqrt(std::max(0.0, ls2 / (double)lum.size() - lmean * lmean));
			if (lstd < 2.5 || lmean < 4.0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(40));
				continue;
			}
		}
		if (g_liveModel == 4) {
			std::string prompt(g_stylePromptBuf);
			if (prompt.empty())prompt = "a detailed realistic animal";
			if (!g_objReady) {
				int rw = std::min(224, fw), rh = std::min(128, fh);
				int offX = std::max(0, (fw - rw) / 2), offY = std::max(0, (fh - rh) / 2);
				std::vector<uint8_t> grayIn((size_t)rw * rh * 4);
				for (size_t i = 0;i < grayIn.size();i += 4) {
					grayIn[i] = 110;grayIn[i + 1] = 110;grayIn[i + 2] = 110;grayIn[i + 3] = 255;
				}
				std::string objPrompt = prompt + ",single subject centered,plain solid light gray background,studio photo,sharp focus,high detail";
				int bw = 448, bh = 256;
				bool foundObj = false;
				int useDs = g_sdPipeline.HasDs() ? 1 : 0;
				g_sdPipeline.SetFastModel(useDs);
				std::vector<uint8_t> lastOut;
				int lastBW = 0, lastBH = 0;
				std::string lastGenErr = "no generation attempted";
				for (int attempt = 0;attempt < 4 && !foundObj;++attempt) {
					if (g_sdCancel.load())break;
					float str = 0.55f + 0.10f * attempt;
					int stps = useDs ? 4 : 6;
					std::vector<uint8_t> bout;
					std::string berr;
					SdStats bst{};
					FastGenCtx fgc{ &g_sdPipeline,&objPrompt,stps,12345 + attempt * 7919,bw,bh,&grayIn,rw,rh,str,&bout,&g_sdCancel,&bst,&berr };
					bool bok = SehCall(SehFastGen, &fgc, berr);
					if (!bok) {
						lastGenErr = berr;
						ObjDbgLog("attempt " + std::to_string(attempt + 1) + " | GEN FAIL | " + berr);
						bool shapeErr = berr.find("status code") != std::string::npos ||
							berr.find("dimension") != std::string::npos ||
							berr.find("shape") != std::string::npos ||
							berr.find("concat") != std::string::npos ||
							berr.find("GPU fault") != std::string::npos;
						if (shapeErr && bw > 256) {
							bw = std::max(256, bw * 3 / 4 / 64 * 64);
							bh = std::max(160, bw * 9 / 16 / 64 * 64);
							ObjDbgLog(" retry at " + std::to_string(bw) + "x" + std::to_string(bh));
							continue;
						}
						if (shapeErr && useDs) {
							useDs = 0;
							g_sdPipeline.SetFastModel(0);
							ObjDbgLog(" switched to TinySD engine");
							continue;
						}
						g_sdStatus = "Object gen failed: " + berr;
						g_liveRun.store(false);
						break;
					}
					lastOut = bout;lastBW = bw;lastBH = bh;
					SaveObjDebug(bout, bw, bh, attempt);
					if (LooksLikeNoise(bout, bw, bh)) {
						ObjDbgLog("attempt " + std::to_string(attempt + 1) + " | REJECTED: static noise");
						g_sdStatus = "Object gen attempt " + std::to_string(attempt + 1) + " | static | retrying";
						lastOut.clear();
						continue;
					}
					{
						double ssum = 0.0, ssum2 = 0.0;size_t scnt = 0;
						for (size_t li = 0;li + 2 < bout.size();li += 64) {
							float l = bout[li] * 0.114f + bout[li + 1] * 0.587f + bout[li + 2] * 0.299f;
							ssum += l;ssum2 += l * l;++scnt;
						}
						double smean = scnt ? ssum / scnt : 0.0;
						double sstd = scnt ? std::sqrt(std::max(0.0, ssum2 / scnt - smean * smean)) : 0.0;
						ObjDbgLog("attempt " + std::to_string(attempt + 1) + " | lstd " + std::to_string(sstd) + " | " + berr);
					}
					std::vector<uint8_t> patch;
					int pw2 = 0, ph2 = 0;
					float cxp = 0.0f, cyp = 0.0f;
					if (ExtractObjFromFlat(grayIn, rw, rh, bout, bw, bh, patch, pw2, ph2, cxp, cyp)) {
						std::lock_guard<std::mutex> lk(g_objMtx);
						g_objPatch = std::move(patch);
						g_objPW = pw2;g_objPH = ph2;
						float s = (float)rw / (float)bw;
						g_objCX = (float)offX + cxp * s;
						g_objCY = (float)offY + cyp * s;
						g_objCW = pw2 * s;g_objCH = ph2 * s;
						g_objScale = 1.0f;
						g_objReady = true;
						g_objDirty = true;
						foundObj = true;
						g_sdStatus = "Object generated | tracking at game FPS";
						{
							std::vector<uint8_t> ry;
							int rw2 = 0, rh2 = 0;
							CapLuma(frame, fw, fh, ry, rw2, rh2);
							g_objRefY = std::move(ry);
							g_objRefSet = true;
							g_objRefCX = g_objCX;g_objRefCY = g_objCY;
							g_objRefCW = g_objCW;g_objRefCH = g_objCH;
							g_objRefScale = g_objScale;
							ObjDbgLog("reference anchored at " + std::to_string(g_objCX) + "," + std::to_string(g_objCY));
						}
					}
					else {
						g_sdStatus = "Object gen attempt " + std::to_string(attempt + 1) + " | retrying";
					}
				}
				if (!foundObj && !g_sdCancel.load() && !lastOut.empty() && lastBW >= 64 && lastBH >= 64 && !LooksLikeNoise(lastOut, lastBW, lastBH)) {
					int cx0 = lastBW * 3 / 10, cx1 = lastBW * 7 / 10;
					int cy0 = lastBH * 25 / 100, cy1 = lastBH * 75 / 100;
					int pw2 = cx1 - cx0 + 1, ph2 = cy1 - cy0 + 1;
					if (pw2 >= 48 && ph2 >= 32) {
						std::lock_guard<std::mutex> lk(g_objMtx);
						g_objPatch.assign((size_t)pw2 * ph2 * 4, 0);
						for (int y = 0;y < ph2;++y)
							for (int x = 0;x < pw2;++x) {
								const uint8_t* s2 = &lastOut[((size_t)(cy0 + y) * lastBW + (cx0 + x)) * 4];
								uint8_t* d2 = &g_objPatch[((size_t)y * pw2 + x) * 4];
								d2[0] = s2[0];d2[1] = s2[1];d2[2] = s2[2];d2[3] = 255;
							}
						float s = (float)rw / (float)lastBW;
						g_objPW = pw2;g_objPH = ph2;
						g_objCX = (float)offX + cx0 * s;
						g_objCY = (float)offY + cy0 * s;
						g_objCW = pw2 * s;g_objCH = ph2 * s;
						g_objScale = 1.0f;
						g_objReady = true;g_objDirty = true;foundObj = true;
						g_sdStatus = "Object stamped | debug: " + g_objDbgPath;
						ObjDbgLog("fallback center stamp used");
						{
							std::vector<uint8_t> ry;
							int rw2 = 0, rh2 = 0;
							CapLuma(frame, fw, fh, ry, rw2, rh2);
							g_objRefY = std::move(ry);
							g_objRefSet = true;
							g_objRefCX = g_objCX;g_objRefCY = g_objCY;
							g_objRefCW = g_objCW;g_objRefCH = g_objCH;
							g_objRefScale = g_objScale;
						}
					}
				}
				if (!foundObj && !g_sdCancel.load()) {
					g_sdStatus = "Object gen failed | log: " + g_objDbgPath;
					ObjDbgLog("FINAL FAIL | last gen error: " + lastGenErr);
					g_liveRun.store(false);
					break;
				}
				if (!foundObj)break;
			}
			std::vector<uint8_t> cy;
			int cw2 = 0, ch2 = 0;
			CapLuma(frame, fw, fh, cy, cw2, ch2);
			{
				std::lock_guard<std::mutex> lk(g_objMtx);
				if (g_objRefSet && g_objRefY.size() == cy.size() && cw2 > 0) {
					float dx = 0.0f, dy = 0.0f;
					EstimateMotion(cy, g_objRefY, cw2, ch2, dx, dy);
					float sx = (float)fw / (float)cw2, syf = (float)fh / (float)ch2;
					g_objCX = g_objRefCX - dx * sx;
					g_objCY = g_objRefCY - dy * syf;
					g_objCW = g_objRefCW;
					g_objCH = g_objRefCH;
					g_objScale = g_objRefScale;
				}
			}
			g_liveStats.framesDone++;
			continue;
		}
		auto tInf0 = std::chrono::steady_clock::now();
		std::vector<uint8_t> out;
		std::string err;
		int W, H;
		if (g_liveModel == 2) {
			W = 512;H = 288;
		}
		else {
			W = std::max(64, (g_liveRes / 64) * 64);
			H = std::max(64, ((W * 9 / 16 + 32) / 64) * 64);
		}
		std::string prompt(g_stylePromptBuf);
		bool anchorWanted = g_anchorOn || PromptHasObject(prompt);
		if (g_anchorReady && anchorWanted) {
			std::vector<uint8_t> cy;
			int cw = 0, ch = 0;
			CapLuma(frame, fw, fh, cy, cw, ch);
			if (g_prevCapY.size() == cy.size()) {
				float dx = 0.0f, dy = 0.0f;
				EstimateMotion(cy, g_prevCapY, cw, ch, dx, dy);
				float sx = (float)W / (float)cw;
				float syf = (float)H / (float)ch;
				g_anchorX -= dx * sx;
				g_anchorY -= dy * syf;
				if (g_anchorBaseDepth > 0 && g_depthEnginePtr) {
					std::vector<float> dmap;
					double dms = 0.0;
					if (g_depthEnginePtr->TryGetResult(dmap, dms) && !dmap.empty()) {
						int ds = g_depthEnginePtr->Size();
						if (ds > 0) {
							int dxx = std::clamp((int)(g_anchorX / (float)W * ds), 0, ds - 1);
							int dyy = std::clamp((int)(g_anchorY / (float)H * ds), 0, ds - 1);
							int curD = (int)(dmap[(size_t)dyy * ds + dxx] * 255.0f);
							if (curD > 10 && g_anchorBaseDepth > 10) {
								float s = (float)g_anchorBaseDepth / (float)curD;
								g_anchorScale = std::clamp(g_anchorScale * s, 0.4f, 3.0f);
							}
						}
					}
				}
			}
			g_prevCapY = std::move(cy);
		}
		SdStats st{};
		bool ok = false;
		if (g_liveModel == 2) {
			MbGenCtx mgc{ &g_mbEngine,&frame,fw,fh,g_liveSteps,12345,&out,&W,&H,&st.unetMs,&err,&g_sdCancel };
			ok = SehCall(SehMbGen, &mgc, err);
			if (ok) {
				double sum = 0.0, sum2 = 0.0;
				size_t cnt = 0;
				for (size_t i = 0;i + 2 < out.size();i += 64) {
					float l = out[i] * 0.114f + out[i + 1] * 0.587f + out[i + 2] * 0.299f;
					sum += l;sum2 += l * l;++cnt;
				}
				if (cnt) {
					double mean = sum / (double)cnt;
					st.outStd = (float)std::sqrt(std::max(0.0, sum2 / (double)cnt - mean * mean));
				}
			}
		}
		else if (g_liveModel == 3) {
			const std::string neg = "blurry,lowres,bad anatomy,jpeg artifacts,watermark";
			FullGenCtx fgc{ &g_sdPipeline,&prompt,&neg,std::max(g_liveSteps,6),7.5f,12345,W,H,
			&frame,fw,fh,g_liveStrength,&out,&g_sdCancel,&st,&err };
			ok = SehCall(SehFullGen, &fgc, err);
			if (!ok && !err.empty() &&
				(err.find("status code") != std::string::npos ||
					err.find("dimension") != std::string::npos ||
					err.find("shape") != std::string::npos ||
					err.find("concat") != std::string::npos)) {
				int oldW = W, oldH = H;
				W = std::max(64, (W / 2) / 64 * 64);
				H = std::max(64, ((W * 9 / 16 + 32) / 64) * 64);
				if (W < oldW) {
					g_sdStatus = "Rendering at " + std::to_string(W) + "x" + std::to_string(H) + " | engine: " + g_sdPipeline.EpName();
					err.clear();
					FullGenCtx fgc2{ &g_sdPipeline,&prompt,&neg,std::max(g_liveSteps,6),7.5f,12345,W,H,
					&frame,fw,fh,g_liveStrength,&out,&g_sdCancel,&st,&err };
					ok = SehCall(SehFullGen, &fgc2, err);
				}
			}
		}
		else {
			FastGenCtx fgc{ &g_sdPipeline,&prompt,g_liveSteps,12345,W,H,&frame,fw,fh,g_liveStrength,&out,&g_sdCancel,&st,&err };
			ok = SehCall(SehFastGen, &fgc, err);
			if (!ok && !err.empty() &&
				(err.find("status code") != std::string::npos ||
					err.find("dimension") != std::string::npos ||
					err.find("shape") != std::string::npos)) {
				W = std::max(64, (g_liveRes / 64) * 64);
				H = std::max(64, ((W * 9 / 16 + 32) / 64) * 64);
				g_sdStatus = "Rendering at " + std::to_string(W) + "x" + std::to_string(H) + " | engine: " + g_sdPipeline.EpName();
				err.clear();
				FastGenCtx fgc2{ &g_sdPipeline,&prompt,g_liveSteps,12345,W,H,&frame,fw,fh,g_liveStrength,&out,&g_sdCancel,&st,&err };
				ok = SehCall(SehFastGen, &fgc2, err);
			}
		}
		if (!ok && g_liveModel == 2 && g_mbEngine.OnGpu() && !g_mbEngine.CpuOnly() &&
			err.find("GPU fault") != std::string::npos) {
			g_mbEngine.Release();
			g_mbEngine.SetCpuOnly();
			g_liveGpuOnly = false;
			std::string me;
			if (g_mbEngine.Init(me)) {
				g_sdStatus = "Moebius | CPU | GPU fault fallback";
				g_liveLastErr.clear();
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}
			err = me.empty() ? "moebius cpu reload failed" : me;
		}
		if (!ok) {
			if (!g_sdCancel.load()) {
				g_liveLastErr = err;
				g_sdStatus = "Live mode error: " + err;
				g_liveRun.store(false);
			}
			break;
		}
		if (st.outStd < 0.015f) {
			g_liveLastErr = "Frame rejected(low detail)| keeping previous frame";
			g_sdStatus = g_liveLastErr;
			g_liveStats.framesSkipped++;
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			continue;
		}
		bool upWanted = (g_liveUpscale || g_liveHd != 0) && !g_upGiveUp.load();
		int upMode = (g_liveHd != 0) ? 2 : g_liveUpMode;
		if (upWanted && upMode == 1 && W >= 512 && g_liveHd == 0) {
			upWanted = false;
			if (!upSkipNote) { upSkipNote = true;g_sdStatus = "512 output | single pass upscaler skipped"; }
		}
		if (upWanted && !g_sdUpscaler.IsReady()) {
			if (g_upPending) {
				if (g_upProv.State() == ProvState::Ready) {
					g_upPending = false;
					g_sdUpscalerLoad();
					if (!g_sdUpscaler.IsReady()) { g_upGiveUp.store(true);g_sdStatus = "Upscaler unavailable | RETRY UPSCALER: " + g_sdUpscaler.LastErr(); }
				}
				else if (g_upProv.State() == ProvState::Failed) {
					g_upPending = false;
					g_upGiveUp.store(true);
					g_sdStatus = "Upscaler download failed: " + g_upProv.Err() + " | RETRY UPSCALER";
				}
			}
			else {
				std::wstring upPath = L"models\\real_esrgan_x4plus-onnx-float\\real_esrgan_x4plus.onnx";
				if (ValidateModelFile(upPath, 400000)) {
					g_sdUpscalerLoad();
					if (!g_sdUpscaler.IsReady()) { g_upGiveUp.store(true);g_sdStatus = "Upscaler unavailable | RETRY UPSCALER: " + g_sdUpscaler.LastErr(); }
				}
				else {
					const AiModelDef& def = kAiModels[0];
					g_upProv.Reset();
					g_upProv.Begin({ def.url1 }, def.localPath, GetAiModelMinBytes(def));
					g_upPending = true;
					g_sdStatus = "Downloading upscaler(~62 MB)...";
				}
			}
		}
		if (upWanted && g_sdUpscaler.IsReady()) {
			if (!g_sdUpscaler.OnGpu()) {
				if (!upSkipNote) { upSkipNote = true;g_sdStatus = "Upscaler on CPU | skipped | GPU upscale only"; }
				upWanted = false;
			}
		}
		if (upWanted && g_sdUpscaler.IsReady()) {
			std::vector<uint8_t> up;
			int uw = 0, uh = 0;
			double upMs = 0;
			bool upOk = (upMode == 2) ? g_sdUpscalerRunTiled(out, W, H, up, uw, uh, &upMs)
				: g_sdUpscalerRun(out, W, H, up, uw, uh, &upMs);
			if (upOk) {
				out = std::move(up);W = uw;H = uh;
				st.upMs = upMs;
			}
			else {
				g_liveLastErr = g_sdUpscalerErr();
			}
		}
		if (W > 2 && H > 2 && (int64_t)W * H <= 1500000) {
			std::vector<uint8_t> shp = out;
			const float amt = 0.35f;
			for (int y = 1;y < H - 1;++y) {
				const uint8_t* r0 = &shp[((size_t)(y - 1) * W) * 4];
				const uint8_t* r1 = &shp[((size_t)y * W) * 4];
				const uint8_t* r2 = &shp[((size_t)(y + 1) * W) * 4];
				uint8_t* d = &out[((size_t)y * W) * 4];
				for (int x = 1;x < W - 1;++x) {
					for (int c = 0;c < 3;++c) {
						int blur = (r0[(x - 1) * 4 + c] + r0[x * 4 + c] + r0[(x + 1) * 4 + c] +
							r1[(x - 1) * 4 + c] + r1[x * 4 + c] + r1[(x + 1) * 4 + c] +
							r2[(x - 1) * 4 + c] + r2[x * 4 + c] + r2[(x + 1) * 4 + c]) / 9;
						int v = r1[x * 4 + c] + (int)(amt * (r1[x * 4 + c] - blur));
						d[x * 4 + c] = (uint8_t)std::clamp(v, 0, 255);
					}
				}
			}
		}
		if (anchorWanted) {
			if (!g_anchorReady && g_anchorPatch.empty()) {
				if (PromptHasObject(prompt)) {
					AnchorInitFromDiff(frame, fw, fh, out, W, H);
					if (g_anchorReady) {
						std::vector<float> dmap;
						double dms = 0.0;
						g_anchorBaseDepth = 0;
						if (g_depthEnginePtr && g_depthEnginePtr->TryGetResult(dmap, dms) && !dmap.empty()) {
							int ds = g_depthEnginePtr->Size();
							if (ds > 0) {
								int dxx = std::clamp((int)(g_anchorX / (float)W * ds), 0, ds - 1);
								int dyy = std::clamp((int)(g_anchorY / (float)H * ds), 0, ds - 1);
								g_anchorBaseDepth = (int)(dmap[(size_t)dyy * ds + dxx] * 255.0f);
							}
						}
						g_sdStatus = "Object anchored to world position";
					}
				}
			}
			AnchorComposite(out, W, H);
		}
		g_liveStats.teMs = st.teMs;
		g_liveStats.encMs = st.encMs;
		g_liveStats.unetMs = st.unetMs;
		g_liveStats.decMs = st.decMs;
		g_liveStats.upMs = st.upMs;
		g_liveStats.outMean = st.outMean;
		g_liveStats.outStd = st.outStd;
		g_liveStats.totalMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tInf0).count();
		{
			std::lock_guard<std::mutex> lk(g_liveOutMtx);
			if (g_livePrevOut.size() == out.size()) {
				float nw = (g_liveStats.totalMs > 900.0) ? 0.78f : (g_liveStats.totalMs > 500.0 ? 0.68f : 0.6f);
				float pw = 1.0f - nw;
				for (size_t bi = 0;bi < out.size();bi += 4) {
					out[bi + 0] = (uint8_t)(out[bi + 0] * nw + g_livePrevOut[bi + 0] * pw + 0.5f);
					out[bi + 1] = (uint8_t)(out[bi + 1] * nw + g_livePrevOut[bi + 1] * pw + 0.5f);
					out[bi + 2] = (uint8_t)(out[bi + 2] * nw + g_livePrevOut[bi + 2] * pw + 0.5f);
				}
			}
			g_livePrevOut = out;
			g_liveOuts[g_liveOutSlot] = std::move(out);
			g_liveOutSlot = (g_liveOutSlot + 1) % 3;
			g_liveOutW = W;g_liveOutH = H;
			g_liveOutReady = true;
			g_liveOutVer.store(ver);
		}
		if (g_liveHd == 0 && g_liveStats.totalMs > 1500.0) {
			if (g_liveSteps > 1 && !g_autoStepped) {
				g_autoStepped = true;
				g_liveSteps--;
				g_sdStatus = "Frame " + std::to_string((int)g_liveStats.totalMs) + " ms | steps " + std::to_string(g_liveSteps + 1) + " -> " + std::to_string(g_liveSteps);
			}
			else if (!g_autoDropped && g_liveRes > 128) {
				g_autoDropped = true;
				int oldRes = g_liveRes;
				g_liveRes = (g_liveRes >= 512) ? 320 : (g_liveRes >= 320) ? 256 : (g_liveRes >= 256) ? 192 : 128;
				g_sdStatus = "Frame " + std::to_string((int)g_liveStats.totalMs) + " ms | res " + std::to_string(oldRes) + " -> " + std::to_string(g_liveRes);
			}
		}
		double dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - tFrame0).count();
		double fps = dt > 0.0 ? 1.0 / dt : 0.0;
		double cur = g_liveFps.load();
		g_liveFps.store(cur > 0.0 ? cur * 0.7 + fps * 0.3 : fps);
		g_liveStats.framesDone++;
	}
	g_liveFps.store(0.0);
	g_liveThreadDone.store(true);
}
static void WaitLiveThread(int seconds) {
	for (int i = 0;i < seconds * 200 && !g_liveThreadDone.load();++i)
		Sleep(5);
}
static void LiveStart() {
	if (g_liveRun.load())return;
	WaitLiveThread(30);
	if (!g_liveThreadDone.load()) {
		g_sdStatus = "Engine is shutting down | try again in a few seconds";
		return;
	}
	if (g_liveThread.joinable())g_liveThread.join();
	g_liveThreadDone.store(false);
	g_sdCancel.store(false);
	g_liveFps.store(0.0);
	g_autoDropped = false;
	g_autoStepped = false;
	if (g_liveModel == 4) {
		g_objReady.store(false);
		g_objDirty.store(false);
		g_objPrevY.clear();
		g_objRefSet = false;
		g_objRefY.clear();
	}
	g_liveRun.store(true);
	g_liveThread = std::thread(LiveWorker);
	g_sdStatus = "Live mode | engine: " + g_sdPipeline.EpName();
}
static void LiveStop(AppConfig& cfg) {
	g_liveRun.store(false);
	g_sdCancel.store(true);
	WaitLiveThread(60);
	if (g_liveThread.joinable() && g_liveThreadDone.load())g_liveThread.join();
	cfg.fx.upscaleSD = false;
	g_sdApplied = false;
	SaveConfig(cfg);
	g_sdStatus = "Live mode stopped.";
}
static void LivePublishFrame(CompositingPipeline& comp) {
	if (!comp.GetScene())return;
	if (g_liveEnabled && g_livePubBusy.load())return;
	if (!comp.DownscaleFrame(g_dev, g_ctx, comp.GetScene(), kLiveDsW, kLiveDsH))return;
	if (!g_liveStaging || g_liveStagingW != kLiveDsW || g_liveStagingH != kLiveDsH) {
		g_liveStaging.Reset();
		D3D11_TEXTURE2D_DESC st{};
		st.Width = kLiveDsW;st.Height = kLiveDsH;st.MipLevels = 1;st.ArraySize = 1;
		st.Format = DXGI_FORMAT_B8G8R8A8_UNORM;st.SampleDesc.Count = 1;
		st.Usage = D3D11_USAGE_STAGING;st.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		if (FAILED(g_dev->CreateTexture2D(&st, nullptr, &g_liveStaging)))return;
		g_liveStagingW = kLiveDsW;g_liveStagingH = kLiveDsH;
	}
	g_ctx->CopyResource(g_liveStaging.Get(), comp.GetDownscaled());
	D3D11_MAPPED_SUBRESOURCE mp{};
	if (SUCCEEDED(g_ctx->Map(g_liveStaging.Get(), 0, D3D11_MAP_READ, 0, &mp)) && mp.pData) {
		int fw = (int)g_liveStagingW, fh = (int)g_liveStagingH;
		std::vector<uint8_t> bytes((size_t)fw * fh * 4);
		for (int y = 0;y < fh;++y)
			memcpy(&bytes[(size_t)y * fw * 4], (const uint8_t*)mp.pData + (size_t)y * mp.RowPitch, (size_t)fw * 4);
		g_ctx->Unmap(g_liveStaging.Get(), 0);
		{
			std::lock_guard<std::mutex> lk(g_liveFrameMtx);
			static std::vector<uint8_t> s_prev;
			g_pubChanged.store(s_prev.size() != bytes.size() || !std::equal(s_prev.begin(), s_prev.end(), bytes.begin()));
			s_prev = bytes;
			g_liveFrameBytes = std::move(bytes);
			g_liveFW = fw;
			g_liveFH = fh;
		}
		g_liveFrameVer.store(g_liveFrameVer.load() + 1);
		if (g_liveEnabled)g_livePubBusy.store(true);
	}
	else {
		g_ctx->Unmap(g_liveStaging.Get(), 0);
	}
}
static std::string LiveEngineName() {
	if (g_liveModel == 2)return std::string("Moebius ") + (g_mbEngine.OnGpu() ? "(GPU)" : "(CPU)");
	return g_sdPipeline.EpName();
}
static void CheckGpuInfo() {
	if (g_gpuInfoChecked)return;
	g_gpuInfoChecked = true;
	try {
		DXGI_ADAPTER_DESC1 ad{};
		if (g_dxgiAdapter && SUCCEEDED(g_dxgiAdapter->GetDesc1(&ad))) {
			std::wstring w(ad.Description);
			g_gpuName = std::string(w.begin(), w.end());
			g_nvidiaGpu = (g_gpuName.find("NVIDIA") != std::string::npos);
		}
		if (g_nvidiaGpu) {
			HKEY hk = nullptr;
			if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{B2FE1952-0186-46C3-BAEC-A80AA35AC5B8}_Display.Driver",
				0, KEY_READ, &hk) == ERROR_SUCCESS) {
				wchar_t buf[64] = {};
				DWORD sz = sizeof(buf), typ = 0;
				if (RegQueryValueExW(hk, L"DisplayDriver", nullptr, &typ, (LPBYTE)buf, &sz) == ERROR_SUCCESS) {
					std::wstring wd(buf);
					g_nvMaj = _wtoi(wd.c_str());
					size_t dot = wd.find(L'.');
					if (dot != std::wstring::npos)g_nvMin = _wtoi(wd.c_str() + dot + 1);
				}
				RegCloseKey(hk);
			}
		}
	}
	catch (...) {}
}
static std::string GpuWarningText() {
	std::string s;
	if (g_nvidiaGpu) {
		if (g_nvMaj > 0 && g_nvMaj < 570) {
			s = "NVIDIA driver " + std::to_string(g_nvMaj) + "." + (g_nvMin < 10 ? "0" : "") + std::to_string(g_nvMin) +
				" | update to 570+ for DirectML support";
		}
		else if (g_nvMaj >= 570) {
			s = "NVIDIA driver " + std::to_string(g_nvMaj) + "." + (g_nvMin < 10 ? "0" : "") + std::to_string(g_nvMin) + " | DirectML ready";
		}
	}
	return s;
}
static bool ExtractObjFromFlat(const std::vector<uint8_t>& flat, int fw, int fh,
	const std::vector<uint8_t>& out, int ow, int oh,
	std::vector<uint8_t>& patch, int& pw, int& ph,
	float& cxp, float& cyp) {
	patch.clear();pw = ph = 0;cxp = cyp = 0.0f;
	if (fw < 8 || fh < 8 || ow < 8 || oh < 8)return false;
	(void)flat;
	const int BORD = 6;
	std::vector<int> rv, gv, bv;
	rv.reserve((size_t)2 * (ow + oh) * BORD * 2);
	auto add = [&](int x, int y) {
		const uint8_t* p = &out[((size_t)y * ow + x) * 4];
		rv.push_back(p[0]);gv.push_back(p[1]);bv.push_back(p[2]);
		};
	for (int y = 0;y < BORD;++y)for (int x = 0;x < ow;++x)add(x, y);
	for (int y = oh - BORD;y < oh;++y)for (int x = 0;x < ow;++x)add(x, y);
	for (int y = BORD;y < oh - BORD;++y) {
		for (int x = 0;x < BORD;++x)add(x, y);
		for (int x = ow - BORD;x < ow;++x)add(x, y);
	}
	std::sort(rv.begin(), rv.end());std::sort(gv.begin(), gv.end());std::sort(bv.begin(), bv.end());
	int bgR = rv[rv.size() / 2], bgG = gv[gv.size() / 2], bgB = bv[bv.size() / 2];
	int gw = std::min(fw, 96), gh = std::min(fh, 64);
	std::vector<float> dist((size_t)gw * gh, 0.0f);
	float maxD = 0.0f;
	for (int gy = 0;gy < gh;++gy) {
		int oy = (int)((gy + 0.5f) * oh / gh);
		for (int gx = 0;gx < gw;++gx) {
			int ox = (int)((gx + 0.5f) * ow / gw);
			const uint8_t* o = &out[((size_t)oy * ow + ox) * 4];
			float d = (float)(std::abs((int)o[0] - bgR) + std::abs((int)o[1] - bgG) + std::abs((int)o[2] - bgB));
			dist[(size_t)gy * gw + gx] = d;
			if (d > maxD)maxD = d;
		}
	}
	if (maxD < 14.0f)return false;
	float thr = std::max(20.0f, maxD * 0.35f);
	int best = 0;
	for (size_t i = 1;i < dist.size();++i)if (dist[i] > dist[best])best = (int)i;
	std::vector<int> stack;stack.push_back(best);
	std::vector<uint8_t> in((size_t)gw * gh, 0);
	int minx = gw, miny = gh, maxx = -1, maxy = -1, cnt = 0;
	while (!stack.empty()) {
		int c = stack.back();stack.pop_back();
		if (in[c])continue;
		in[c] = 1;
		int gx = c % gw, gy = c / gw;
		minx = std::min(minx, gx);maxx = std::max(maxx, gx);
		miny = std::min(miny, gy);maxy = std::max(maxy, gy);
		++cnt;
		auto push = [&](int nx, int ny) {
			if (nx < 0 || ny < 0 || nx >= gw || ny >= gh)return;
			size_t nc = (size_t)ny * gw + nx;
			if (!in[nc] && dist[nc] >= thr)stack.push_back((int)nc);
			};
		push(gx - 1, gy);push(gx + 1, gy);push(gx, gy - 1);push(gx, gy + 1);
	}
	float areaFrac = (float)cnt / (float)(gw * gh);
	if (areaFrac > 0.7f) {
		minx = gw / 4;maxx = gw * 3 / 4;miny = gh / 3;maxy = gh * 2 / 3;
		cnt = 0;
		for (int gy = miny;gy <= maxy;++gy)
			for (int gx = minx;gx <= maxx;++gx)
				if (dist[(size_t)gy * gw + gx] >= thr)++cnt;
		if (cnt < 8)return false;
	}
	else if (cnt < 4) {
		return false;
	}
	int wpx = maxx - minx + 1, hpx = maxy - miny + 1;
	if (std::max(wpx, hpx) > 5 * std::min(wpx, hpx) + 8)return false;
	float oxf = (float)ow / (float)gw, oyf = (float)oh / (float)gh;
	int bx0 = std::max(0, (int)(minx * oxf) - 6), by0 = std::max(0, (int)(miny * oyf) - 6);
	int bx1 = std::min(ow - 1, (int)((maxx + 1) * oxf) + 6), by1 = std::min(oh - 1, (int)((maxy + 1) * oyf) + 6);
	pw = bx1 - bx0 + 1;ph = by1 - by0 + 1;
	if (pw < 24 || ph < 24)return false;
	int cov = 0;
	for (int y = 0;y < ph;++y) {
		int oy2 = by0 + y;
		for (int x = 0;x < pw;++x) {
			int ox2 = bx0 + x;
			const uint8_t* o = &out[((size_t)oy2 * ow + ox2) * 4];
			float d = (float)(std::abs((int)o[0] - bgR) + std::abs((int)o[1] - bgG) + std::abs((int)o[2] - bgB));
			if (d >= thr)++cov;
		}
	}
	if (cov < (pw* ph) / 60)return false;
	float adiv = maxD > 0.0f ? 1.0f / maxD : 1.0f;
	patch.assign((size_t)pw * ph * 4, 0);
	for (int y = 0;y < ph;++y) {
		int oy2 = by0 + y;
		for (int x = 0;x < pw;++x) {
			int ox2 = bx0 + x;
			const uint8_t* o = &out[((size_t)oy2 * ow + ox2) * 4];
			float d = (float)(std::abs((int)o[0] - bgR) + std::abs((int)o[1] - bgG) + std::abs((int)o[2] - bgB));
			float a = std::clamp((d - 16.0f) * adiv, 0.0f, 1.0f);
			uint8_t* p = &patch[((size_t)y * pw + x) * 4];
			p[0] = o[0];p[1] = o[1];p[2] = o[2];p[3] = (uint8_t)(a * 255.0f);
		}
	}
	cxp = (float)bx0;cyp = (float)by0;
	return true;
}
static void LoadGameName() {
	std::ifstream f("models\\game_name.txt");
	if (f) { std::getline(f, g_gameName); }
}
static void SaveGameName() {
	std::error_code ec;
	std::filesystem::create_directories(L"models", ec);
	std::ofstream f("models\\game_name.txt", std::ios::trunc);
	if (f)f << g_gameName;
}
static bool YoloReady() {
	const SdFileSpec* s = &kVisionFiles[g_radarModelIdx];
	return ValidateModelFile(s->localPath, s->minBytes);
}
static SdProvisioner g_radarProv;
static void RadarWorkerBody();
static bool SehRadarWorkerBody(void* p);
static void RadarWorker() {
	std::string sehErr;
	SehCall(SehRadarWorkerBody, nullptr, sehErr);
	g_radarThreadDone.store(true);
}
static bool SehRadarWorkerBody(void*) {
	RadarWorkerBody();
	return true;
}
static void RadarWorkerBody() {
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	while (!YoloReady() && g_radarRun.load()) {
		if (!g_radarProv.Running() && g_radarProv.State() == SdProvState::Failed) {
			g_radarErr = "yolo download failed: " + g_radarProv.Err();
			g_radarRun.store(false);
			g_radarThreadDone.store(true);
			return;
		}
		if (!g_radarProv.Running() && !YoloReady()) {
			g_radarProv.Reset();
			g_radarProv.Start(&kVisionFiles[g_radarModelIdx], 1);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	if (!YoloReady()) {
		g_radarErr = "yolo model missing";
		g_radarRun.store(false);
		g_radarThreadDone.store(true);
		return;
	}
	if (!g_vision.IsReady()) {
		std::string err;
		g_sdStatus = "Loading detector...";
		bool initOk = false;
		{
			std::lock_guard<std::mutex> onk(g_onnxMtx);
			initOk = g_vision.LoadYoloOnly(err);
		}
		if (!initOk) {
			g_radarErr = err;
			g_radarYoloOk.store(-1);
			g_radarRun.store(false);
			g_radarThreadDone.store(true);
			return;
		}
	}
	g_radarYoloOk.store(1);
	g_radarErr.clear();
	g_radarDets.clear();
	g_radarFW = 0;g_radarFH = 0;
	g_radarRawDets.store(0);
	g_radarYoloOk.store(g_vision.IsReady() ? 1 : 0);
	g_sdStatus = "Radar engine running | detecting players...";
	int lastVer = -1;
	std::vector<uint8_t> lastFrame;
	int lastFW = 0, lastFH = 0;
	while (g_radarRun.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		int ver = g_radarFrameVer.load();
		bool got = false;
		if (ver != lastVer) {
			lastVer = ver;
			std::lock_guard<std::mutex> lk(g_radarFrameMtx);
			if (!g_radarFrameBytes.empty()) { lastFrame = g_radarFrameBytes;lastFW = g_radarFrameW;lastFH = g_radarFrameH;got = true; }
		}
		if (!got && !lastFrame.empty()) { got = true; }
		if (!got) {
			std::lock_guard<std::mutex> lk(g_liveFrameMtx);
			if (!g_liveFrameBytes.empty()) { lastFrame = g_liveFrameBytes;lastFW = g_liveFW;lastFH = g_liveFH;got = true; }
		}
		if (!got || lastFrame.empty())continue;
		std::vector<uint8_t> frame = lastFrame;
		int fw = lastFW, fh = lastFH;
		{
			static int dbgTick = 0;
			if ((++dbgTick % 15) == 0) {
				std::ofstream dv("models\\radar_view.bmp", std::ios::binary);
				if (dv) {
					uint32_t rw2 = ((uint32_t)fw * 3 + 3) & ~3u;
					uint32_t ds2 = rw2 * (uint32_t)fh;
					uint8_t hd[54] = {};
					hd[0] = 'B';hd[1] = 'M';
					uint32_t fs2 = 54 + ds2;
					memcpy(hd + 2, &fs2, 4);hd[10] = 54;hd[14] = 40;
					memcpy(hd + 18, &fw, 4);memcpy(hd + 22, &fh, 4);
					hd[26] = 1;hd[28] = 24;
					memcpy(hd + 34, &ds2, 4);
					dv.write((const char*)hd, 54);
					std::vector<uint8_t> row(rw2, 0);
					for (int y = fh - 1;y >= 0;--y) {
						for (int x = 0;x < fw;++x) {
							const uint8_t* p = &frame[((size_t)y * fw + x) * 4];
							row[(size_t)x * 3 + 0] = p[2];
							row[(size_t)x * 3 + 1] = p[1];
							row[(size_t)x * 3 + 2] = p[0];
						}
						dv.write((const char*)row.data(), (std::streamsize)rw2);
					}
				}
			}
		}
		std::vector<VisionDetection> dets;
		g_radarLastFrameT.store(std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count());
		try {
			const SdFileSpec* mspec = &kVisionFiles[g_radarModelIdx];
			std::lock_guard<std::mutex> onk(g_onnxMtx);
			if (g_vision.YoloPath() != mspec->localPath) {
				if (ValidateModelFile(mspec->localPath, mspec->minBytes))
					g_vision.SetYoloModel(mspec->localPath);
			}
			if (g_vision.RunYolo(frame, fw, fh, dets, g_radarConf, 0.35f, 640, g_radarZoom,
				g_radarFocusX.load(), g_radarFocusY.load())) {
				g_radarRawDets.store((int)dets.size());
				std::vector<VisionDetection> people;
				bool onlyP = g_radarPlayersOnly;
				float minW = (float)fw * 0.01f, minH = (float)fh * 0.015f;
				float maxW = (float)fw * 0.9f, maxH = (float)fh * 0.95f;
				for (auto& d : dets) {
					if (onlyP && d.cls != 0)continue;
					if (d.w < minW || d.h < minH)continue;
					if (d.w > maxW || d.h > maxH)continue;
					if (d.h < d.w * 0.5f)continue;
					people.push_back(d);
				}
				if (people.size() > 32)people.resize(32);
				struct RadarTrack { VisionDetection d;int age = 0;int lost = 0; };
				static std::vector<RadarTrack> s_tracks;
				static float s_focusX = 0.5f, s_focusY = 0.5f;
				std::vector<VisionDetection> confirmed;
				for (auto& c : people) {
					int bestT = -1;float bestD = 1e9f;
					for (int t = 0;t < (int)s_tracks.size();++t) {
						float ddx = c.cx - s_tracks[(size_t)t].d.cx, ddy = c.cy - s_tracks[(size_t)t].d.cy;
						float dd = ddx * ddx + ddy * ddy;
						float maxD = std::max(c.w, s_tracks[(size_t)t].d.w);
						if (dd < maxD * maxD * 0.5f && dd < bestD) { bestD = dd;bestT = t; }
					}
					if (bestT >= 0) {
						RadarTrack& t = s_tracks[(size_t)bestT];
						t.d.cx = t.d.cx * 0.55f + c.cx * 0.45f;
						t.d.cy = t.d.cy * 0.55f + c.cy * 0.45f;
						t.d.w = t.d.w * 0.6f + c.w * 0.4f;
						t.d.h = t.d.h * 0.6f + c.h * 0.4f;
						t.d.conf = std::max(t.d.conf, c.conf);
						t.age++;
						t.lost = 0;
					}
					else if ((int)s_tracks.size() < 48) {
						RadarTrack nt;nt.d = c;nt.age = 1;nt.lost = 0;
						s_tracks.push_back(nt);
					}
				}
				for (int t = 0;t < (int)s_tracks.size();) {
					RadarTrack& tr = s_tracks[(size_t)t];
					if (tr.lost > 0) {
						tr.lost++;
						if (tr.lost > 3) { s_tracks.erase(s_tracks.begin() + t);continue; }
					}
					else {
						tr.lost = 1;
					}
					if (tr.age >= 2 && tr.d.conf >= g_radarConf)confirmed.push_back(tr.d);
					++t;
				}
				if (!confirmed.empty()) {
					int bestC = 0;float bestConf = -1.0f;
					for (int t = 0;t < (int)confirmed.size();++t)
						if (confirmed[(size_t)t].conf > bestConf) { bestConf = confirmed[(size_t)t].conf;bestC = t; }
					s_focusX = std::clamp(confirmed[(size_t)bestC].cx / (float)fw, 0.0f, 1.0f);
					s_focusY = std::clamp(confirmed[(size_t)bestC].cy / (float)fh, 0.0f, 1.0f);
				}
				std::lock_guard<std::mutex> lk(g_radarMtx);
				size_t nPeople = confirmed.size();
				g_radarDets = std::move(confirmed);
				g_radarFW = fw;g_radarFH = fh;
				char dbg[96];
				if (dets.empty())
					sprintf_s(dbg, "Radar: 0 | %dx%d", fw, fh);
				else if (nPeople == 0)
					sprintf_s(dbg, "Radar: 0 players | %dx%d", fw, fh);
				else
					sprintf_s(dbg, "Radar: %zu players | %dx%d", nPeople, fw, fh);
				g_sdStatus = dbg;
				g_radarFocusX.store(s_focusX);g_radarFocusY.store(s_focusY);
			}
			else {
				g_radarErr = g_epBroken.load() ? "GPU output invalid | see models\\sr_gpu.log" : "yolo returned false";
			}
		}
		catch (...) {
			g_radarErr = "yolo run failed";
		}
	}
	g_radarThreadDone.store(true);
}
static void RadarStart();
struct RadarStartCtx {};
static bool SehRadarStart(void*) {
	RadarStart();
	return true;
}
static void RadarStartSafe() {
	std::string sehErr;
	if (!SehCall(SehRadarStart, nullptr, sehErr)) {
		g_sdStatus = "Radar failed to start | GPU fault";
	}
}
static void RadarStart() {
	if (g_radarRun.load())return;
	if (g_radarThread.joinable()) {
		if (g_radarThreadDone.load())g_radarThread.join();
		else {
			g_sdStatus = "Radar is still stopping | wait a moment";
			return;
		}
	}
	std::ofstream gl("models\\sr_gpu.log", std::ios::out);
	if (gl) {
		gl << OrtVersionStr() << "\n" << CudaGpuReport() << "\n";
		gl << "vision ep: " << g_vision.EpName() << "\n";
	}
	if (!YoloReady() && !g_radarProv.Running()) {
		g_radarProv.Reset();
		g_radarProv.Start(&kVisionFiles[g_radarModelIdx], 1);
		g_sdStatus = "Downloading YOLO(10 MB)...";
	}
	g_radarThreadDone.store(false);
	g_radarRun.store(true);
	g_radarThread = std::thread(RadarWorker);
	if (YoloReady())g_sdStatus = "Radar on | detecting players";
}
static void RadarStop() {
	g_radarRun.store(false);
	for (int i = 0;i < 400 && !g_radarThreadDone.load();++i)
		Sleep(5);
	if (g_radarThread.joinable() && g_radarThreadDone.load())g_radarThread.join();
	g_sdStatus = "Player radar stopped";
}
static void DrawKeycap(ImDrawList* dl, ImVec2 pos, ImVec2 sz, const char* label, ImU32 bg, ImU32 fg) {
	dl->AddRectFilled(pos, { pos.x + sz.x,pos.y + sz.y }, bg, 6.0f);
	dl->AddRect(pos, { pos.x + sz.x,pos.y + sz.y }, IM_COL32(255, 255, 255, 70), 6.0f);
	ImVec2 ts = ImGui::CalcTextSize(label);
	dl->AddText({ pos.x + (sz.x - ts.x) * 0.5f,pos.y + (sz.y - ts.y) * 0.5f }, fg, label);
}
static int g_brTokUser = 151644, g_brTokAsst = 151645, g_brTokEnd = 151645, g_brTokEos = 151645;
static std::vector<std::string> Utf8Chars(const std::string& s) {
	std::vector<std::string> out;
	size_t i = 0, n = s.size();
	while (i < n) {
		unsigned char c = (unsigned char)s[i];
		int len = 1;
		if ((c & 0xE0) == 0xC0)len = 2;
		else if ((c & 0xF0) == 0xE0)len = 3;
		else if ((c & 0xF8) == 0xF0)len = 4;
		if (i + len > n)len = 1;
		out.push_back(s.substr(i, len));
		i += len;
	}
	return out;
}
class PhiTok {
public:
	bool Load(const std::string& path, std::string& err) {
		std::ifstream f(path, std::ios::binary);
		if (!f) { err = "tokenizer.json open failed";return false; }
		std::string js((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		sd15::Json doc = sd15::JsonParser(js).parse();
		if (doc.t != sd15::Json::T::Obj) { err = "tokenizer.json not object";return false; }
		const sd15::Json* model = doc.find("model");
		if (!model || model->t != sd15::Json::T::Obj) { err = "no model section";return false; }
		const sd15::Json* vocab = model->find("vocab");
		const sd15::Json* merges = model->find("merges");
		if (!vocab || vocab->t != sd15::Json::T::Obj) { err = "no vocab";return false; }
		m_vocab.clear();m_rev.clear();
		for (auto& kv : vocab->obj) {
			if (kv.second.t == sd15::Json::T::Num) {
				int id = (int)kv.second.n;
				m_vocab[kv.first] = id;
				if ((int)m_rev.size() <= id)m_rev.resize((size_t)id + 1);
				m_rev[id] = kv.first;
			}
		}
		m_merges.clear();
		if (merges && merges->t == sd15::Json::T::Arr) {
			for (auto& m2 : merges->arr) {
				if (m2.t == sd15::Json::T::Str) {
					const std::string& s = m2.s;
					size_t sp = s.find(' ');
					if (sp != std::string::npos)
						m_merges[std::make_pair(s.substr(0, sp), s.substr(sp + 1))] = (int)m_merges.size();
				}
			}
		}
		const sd15::Json* added = doc.find("added_tokens");
		if (added && added->t == sd15::Json::T::Arr) {
			for (auto& t : added->arr) {
				if (t.t == sd15::Json::T::Obj) {
					const sd15::Json* idj = t.find("id");
					const sd15::Json* cj = t.find("content");
					if (idj && cj && idj->t == sd15::Json::T::Num && cj->t == sd15::Json::T::Str) {
						int id = (int)idj->n;
						m_vocab[cj->s] = id;
						if ((int)m_rev.size() <= id)m_rev.resize((size_t)id + 1);
						m_rev[id] = cj->s;
					}
				}
			}
		}
		if (m_vocab.empty()) { err = "empty vocab";return false; }
		return true;
	}
	int PieceId(const std::string& p)const {
		auto it = m_vocab.find(p);
		return it == m_vocab.end() ? -1 : it->second;
	}
	std::vector<int> Encode(const std::string& text, std::string& err) {
		std::vector<int> ids;
		if (m_added.empty()) {
			for (auto& kv : m_vocab) {
				const std::string& k = kv.first;
				if (k.size() >= 2 && k[0] == '<' && k.back() == '>')m_added.push_back(k);
			}
			std::sort(m_added.begin(), m_added.end(),
				[](const std::string& a2, const std::string& b2) {return a2.size() > b2.size();});
		}
		size_t pos = 0;
		while (pos < text.size()) {
			bool matched = false;
			for (auto& sp : m_added) {
				if (text.compare(pos, sp.size(), sp) == 0) {
					int pid = PieceId(sp);
					if (pid >= 0)ids.push_back(pid);
					pos += sp.size();
					matched = true;
					break;
				}
			}
			if (matched)continue;
			size_t next = pos;
			while (next < text.size()) {
				bool atSpecial = false;
				for (auto& sp : m_added)
					if (text.compare(next, sp.size(), sp) == 0) { atSpecial = true;break; }
				if (atSpecial)break;
				++next;
			}
			std::string seg = text.substr(pos, next - pos);
			if (!seg.empty()) {
				std::vector<std::string> pieces = PreTokenize(seg);
				for (auto& piece : pieces) {
					int pid = PieceId(piece);
					if (pid >= 0) { ids.push_back(pid);continue; }
					BpeWord(piece, ids);
				}
			}
			pos = next;
		}
		return ids;
	}
	std::string Decode(const std::vector<int>& ids, bool stripSpecials) {
		std::string out;
		for (int id : ids) {
			if (id < 0 || (size_t)id >= m_rev.size())continue;
			const std::string& t = m_rev[id];
			if (stripSpecials && !t.empty() && t[0] == '<' && t.back() == '>') {
				if (t == "<s>" || t == "</s>" || t == "<unk>")continue;
				continue;
			}
			if (t.rfind("<|", 0) == 0 && t.find("|>") != std::string::npos)continue;
			std::string d = t;
			size_t p = 0;
			while ((p = d.find("\xc4\xa0", p)) != std::string::npos) { d.replace(p, 2, " ");p += 1; }
			out += d;
		}
		return out;
	}
private:
	std::vector<std::string> PreTokenize(const std::string& text) {
		std::vector<std::string> out;
		size_t i = 0, n = text.size();
		bool pendSpace = false;
		auto flushWord = [&](std::string& w, bool sp) {
			if (w.empty())return;
			out.push_back((sp ? "\xc4\xa0" : "") + w);
			w.clear();
			};
		std::string word, punct;
		auto isWordChar = [](unsigned char c) {return c >= 0x80 || isalnum(c) || c == '_' || c == '\'';};
		while (i < n) {
			unsigned char c = (unsigned char)text[i];
			if (c == ' ' || c == '\t') {
				flushWord(word, pendSpace);pendSpace = true;
				flushWord(punct, true);
				i += (c == '\t') ? 1 : 1;
				if (c == '\t') { out.push_back("\xc4\xa0");pendSpace = false; }
				continue;
			}
			if (c == '\n' || c == '\r') {
				flushWord(word, pendSpace);pendSpace = false;
				flushWord(punct, false);
				out.push_back("\xc4\xa0\n");
				i += 1;
				continue;
			}
			if (isWordChar(c)) {
				flushWord(punct, pendSpace);
				word += (char)c;
			}
			else {
				flushWord(word, pendSpace);
				pendSpace = false;
				punct += (char)c;
			}
			i += 1;
		}
		flushWord(word, pendSpace);
		flushWord(punct, pendSpace);
		return out;
	}
	void BpeWord(const std::string& word, std::vector<int>& ids) {
		std::vector<std::string> chars = Utf8Chars(word);
		if (chars.empty())return;
		while (chars.size() > 1) {
			int bestRank = -1;size_t bestIdx = 0;
			for (size_t k = 0;k + 1 < chars.size();++k) {
				auto it = m_merges.find(std::make_pair(chars[k], chars[k + 1]));
				if (it != m_merges.end() && (bestRank < 0 || it->second < bestRank)) {
					bestRank = it->second;bestIdx = k;
				}
			}
			if (bestRank < 0)break;
			chars[bestIdx] = chars[bestIdx] + chars[bestIdx + 1];
			chars.erase(chars.begin() + (long)bestIdx + 1);
		}
		for (auto& c : chars) {
			int pid = PieceId(c);
			if (pid >= 0)ids.push_back(pid);
			else {
				int u = PieceId("<unk>");
				ids.push_back(u >= 0 ? u : 0);
			}
		}
	}
	std::map<std::string, int> m_vocab;
	std::vector<std::string> m_rev;
	std::map<std::pair<std::string, std::string>, int> m_merges;
	std::vector<std::string> m_added;
};
#if SR_HAS_ONNX
class BrainChat {
public:
	bool Init(std::string& err) {
		m_err.clear();
		if (m_ready)return true;
		if (!m_tokLoaded) {
			std::string te;
			if (!m_tok.Load("models\\brain\\tokenizer.json", te)) { err = "tokenizer: " + te;return false; }
			m_tokLoaded = true;
		}
		m_cpuFallback = false;
		m_gpuName = "";
		try {
			Ort::SessionOptions opts;
			opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			opts.SetIntraOpNumThreads(std::min(4u, std::max(2u, std::thread::hardware_concurrency())));
			if (!g_epBroken.load()) {
				if (CudaAvailable() && AppendEpByName(opts, EpCuda))m_gpuName = "CUDA";
				else if (AppendEpByName(opts, EpDml))m_gpuName = "DirectML";
			}
			m_sess = Ort::Session(GetOrtEnv(), L"models\\brain\\qwen2.5-0.5b-fp16.onnx", opts);
		}
		catch (...) {
			try {
				Ort::SessionOptions copts;
				copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				copts.SetIntraOpNumThreads(std::min(4u, std::max(2u, std::thread::hardware_concurrency())));
				m_sess = Ort::Session(GetOrtEnv(), L"models\\brain\\qwen2.5-0.5b-fp16.onnx", copts);
				m_cpuFallback = true;
				m_gpuName = "";
			}
			catch (const std::exception& e) {
				err = std::string("brain load: ") + e.what();
				return false;
			}
		}
		m_ins.clear();
		for (int i = 0;i < (int)m_sess.GetInputCount();++i) {
			BrIn in;
			in.name = OrtSessionInputName(m_sess, i);
			try { in.type = m_sess.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType(); }
			catch (...) { in.type = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED; }
			std::string n = in.name;
			if (n.find("input_ids") != std::string::npos)in.kind = 0;
			else if (n.find("attention_mask") != std::string::npos)in.kind = 1;
			else if (n.find("past") != std::string::npos)in.kind = 2;
			else in.kind = 3;
			m_ins.push_back(in);
		}
		m_outLogits.clear();
		m_pastOut.clear();
		for (int i = 0;i < (int)m_sess.GetOutputCount();++i) {
			std::string n = OrtSessionOutputName(m_sess, i);
			if (n.find("present") != std::string::npos || n.find("past") != std::string::npos)
				m_pastOut.push_back(OrtSessionOutputName(m_sess, i));
			else
				m_outLogits.push_back(OrtSessionOutputName(m_sess, i));
		}
		m_kvCount = (int)m_pastOut.size();
		if (m_ins.empty() || m_outLogits.empty() || m_kvCount < 2) { err = "brain graph layout unexpected";return false; }
		m_ready = true;
		return true;
	}
	bool IsReady()const { return m_ready; }
	bool IsCpu()const { return m_cpuFallback; }
	std::string GpuName()const { return m_gpuName; }
	std::string LastErr()const { return m_err; }
	bool ReloadCpu() {
		if (m_cpuFallback || !m_ready)return m_cpuFallback;
		try {
			Ort::SessionOptions copts;
			copts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			copts.SetIntraOpNumThreads(std::min(4u, std::max(2u, std::thread::hardware_concurrency())));
			Ort::Session s = Ort::Session(GetOrtEnv(), L"models\\brain\\qwen2.5-0.5b-fp16.onnx", copts);
			m_sess = std::move(s);
			m_cpuFallback = true;
			return true;
		}
		catch (...) { return false; }
	}
	bool GenerateImpl(const std::string& system, const std::string& user, int maxNew,
		std::string& outText, std::string& errOut,
		const std::atomic<bool>* cancel, std::atomic<int>* newTokens,
		std::function<void(const std::string&)> onDraft = nullptr) {
		if (!m_ready) { errOut = "brain not loaded";return false; }
		try {
			std::string prompt = "<|im_start|>system\n" + system + "<|im_end|>\n<|im_start|>user\n" + user + "<|im_end|>\n<|im_start|>assistant\n";
			std::vector<int> ids = m_tok.Encode(prompt, errOut);
			if (ids.empty()) { errOut = "encode empty";return false; }
			if ((int)ids.size() > 512)ids.resize(512);
			Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPUInput);
			std::vector<int> gen;
			for (int step = 0;step < maxNew;++step) {
				if (cancel && cancel->load()) { errOut = "cancelled";return false; }
				std::vector<int> full = ids;
				full.insert(full.end(), gen.begin(), gen.end());
				int seq = (int)full.size();
				std::vector<int64_t> idV(full.begin(), full.end());
				std::vector<int64_t> mask((size_t)seq, 1);
				std::vector<int64_t> posV((size_t)seq);
				for (int pi = 0;pi < seq;++pi)posV[pi] = pi;
				std::vector<const char*> inNames;
				std::vector<Ort::Value> inVals;
				std::array<int64_t, 2> idShape{ 1,(int64_t)seq };
				std::array<int64_t, 2> mkShape{ 1,(int64_t)seq };
				std::array<int64_t, 2> posShape{ 1,(int64_t)seq };
				std::array<int64_t, 4> kvShape{ 1,2,0,64 };
				int kvIdx = 0;
				for (auto& in : m_ins) {
					if (in.kind == 0) {
						inNames.push_back(in.name.c_str());
						inVals.emplace_back(Ort::Value::CreateTensor<int64_t>(mi, idV.data(), idV.size(), idShape.data(), idShape.size()));
					}
					else if (in.kind == 1) {
						inNames.push_back(in.name.c_str());
						inVals.emplace_back(Ort::Value::CreateTensor<int64_t>(mi, mask.data(), mask.size(), mkShape.data(), mkShape.size()));
					}
					else if (in.kind == 3) {
						inNames.push_back(in.name.c_str());
						inVals.emplace_back(Ort::Value::CreateTensor<int64_t>(mi, posV.data(), posV.size(), posShape.data(), posShape.size()));
					}
					else {
						if (in.type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16) {
							static uint16_t empty16[1] = { 0 };
							inNames.push_back(in.name.c_str());
							inVals.emplace_back(MakeFp16Tensor(mi, empty16, 0, kvShape.data(), kvShape.size()));
						}
						else if (in.type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
							static int64_t empty64[1] = { 0 };
							inNames.push_back(in.name.c_str());
							inVals.emplace_back(Ort::Value::CreateTensor<int64_t>(mi, empty64, 0, kvShape.data(), kvShape.size()));
						}
						else {
							static float emptyF[1] = { 0.0f };
							inNames.push_back(in.name.c_str());
							inVals.emplace_back(Ort::Value::CreateTensor<float>(mi, emptyF, 0, kvShape.data(), kvShape.size()));
						}
						++kvIdx;
					}
				}
				std::vector<const char*> outNames;
				for (auto& o : m_outLogits)outNames.push_back(o.c_str());
				auto res = m_sess.Run(Ort::RunOptions{ nullptr }, inNames.data(), inVals.data(), (int)inNames.size(), outNames.data(), (int)outNames.size());
				std::vector<float> logits;
				if (ValueIsFp16(res[0])) {
					auto* h = res[0].GetTensorMutableData<uint16_t>();
					size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
					logits.resize(cnt);
					for (size_t bi = 0;bi < cnt;++bi)logits[bi] = Fp16ToF32(h[bi]);
				}
				else {
					const float* d = res[0].GetTensorMutableData<float>();
					size_t cnt = res[0].GetTensorTypeAndShapeInfo().GetElementCount();
					logits.assign(d, d + cnt);
				}
				int vocab = 151936;
				size_t lastRow = ((size_t)seq - 1) * (size_t)vocab;
				if (logits.size() < lastRow + vocab)lastRow = logits.size() - vocab;
				bool logitsOk = true;
				for (int v = 0;v < vocab;++v)if (!std::isfinite(logits[lastRow + v])) { logitsOk = false;break; }
				if (!logitsOk) { g_epBroken.store(true);throw std::runtime_error("gpu logits invalid"); }
				float temp = 0.7f;
				float repPen = 1.12f;
				std::vector<float> probs((size_t)vocab);
				float maxL = -1e30f;
				for (int v = 0;v < vocab;++v)if (logits[lastRow + v] > maxL)maxL = logits[lastRow + v];
				float esum = 0.0f;
				for (int v = 0;v < vocab;++v) {
					float f = (logits[lastRow + v] - maxL) / temp;
					for (int g : gen)if (g == v)f -= repPen;
					if (v == 151643 || v == 151645)f -= 2.0f;
					probs[v] = std::exp(std::clamp(f, -80.0f, 40.0f));
					esum += probs[v];
				}
				if (esum <= 0.0f)esum = 1.0f;
				std::vector<std::pair<float, int>> sorted;
				sorted.reserve((size_t)vocab);
				for (int v = 0;v < vocab;++v)sorted.emplace_back(probs[v] / esum, v);
				std::sort(sorted.begin(), sorted.end(),
					[](const std::pair<float, int>& a, const std::pair<float, int>& b) {return a.first > b.first;});
				float topP = 0.9f;
				float psum = 0.0f;
				int keep = (int)sorted.size() - 1;
				for (int i = 0;i < (int)sorted.size();++i) {
					psum += sorted[(size_t)i].first;
					if (psum >= topP) { keep = i;break; }
				}
				std::mt19937 rng((unsigned)(0x9E3779B9u ^ ((uint32_t)(step + 1) * 2654435761u) ^ (uint32_t)((size_t)this + (size_t)&outText)));
				float r = (float)rng() / (float)0xFFFFFFFF;
				float acc = 0.0f;
				int best = sorted[0].second;
				for (int i = 0;i <= keep;++i) {
					acc += sorted[(size_t)i].first;
					if (r < acc) { best = sorted[(size_t)i].second;break; }
				}
				if (best == 151643 || best == 151645) {
					if ((int)gen.size() >= 6)break;
					int alt = -1;
					for (int i2 = 0;i2 <= keep && i2 < 128;++i2) {
						int v2 = sorted[(size_t)i2].second;
						if (v2 != 151643 && v2 != 151645 && v2 != 151644) { alt = v2;break; }
					}
					if (alt < 0)alt = sorted[0].second;
					if (alt == 151643 || alt == 151645 || alt == 151644)break;
					best = alt;
				}
				gen.push_back(best);
				if (newTokens)newTokens->store((int)gen.size());
				if (onDraft)onDraft(m_tok.Decode(gen, true));
				if ((int)gen.size() >= maxNew)break;
			}
			outText = m_tok.Decode(gen, true);
			return true;
		}
		catch (const std::exception& e) {
			if (!m_cpuFallback) {
				std::string cpuErr;
				if (ReloadCpu()) {
					errOut.clear();
					return GenerateImpl(system, user, maxNew, outText, errOut, cancel, newTokens, onDraft);
				}
			}
			errOut = std::string("brain run: ") + e.what();
			return false;
		}
	}
	bool Generate(const std::string& system, const std::string& user, int maxNew,
		std::string& outText, std::string& errOut,
		const std::atomic<bool>* cancel, std::atomic<int>* newTokens,
		std::function<void(const std::string&)> onDraft = nullptr) {
		if (g_epBroken.load() && !m_cpuFallback)ReloadCpu();
		return GenerateImpl(system, user, maxNew, outText, errOut, cancel, newTokens, onDraft);
	}
	bool Ready()const { return m_ready; }
private:
	struct BrIn {
		std::string name;
		ONNXTensorElementDataType type = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
		int kind = 3;
	};
	PhiTok m_tok;
	bool m_tokLoaded = false;
	Ort::Session m_sess{ nullptr };
	std::vector<BrIn> m_ins;
	std::vector<std::string> m_outLogits, m_pastOut;
	int m_kvCount = 0;
	std::string m_err;
	bool m_ready = false;
	bool m_cpuFallback = false;
	std::string m_gpuName;
};
#else
class BrainChat {
public:
	bool Init(std::string& err) { err = "not built with ONNX";return false; }
	bool IsReady()const { return false; }
	bool IsCpu()const { return true; }
	std::string GpuName()const { return ""; }
	std::string LastErr()const { return "not built with ONNX"; }
	bool Generate(const std::string&, const std::string&, int, std::string&, std::string&, const std::atomic<bool>*, std::atomic<int>*, std::function<void(const std::string&)> = nullptr) { return false; }
	bool Ready()const { return false; }
};
#endif
static BrainChat g_brain;
static std::atomic<bool> g_brainRun{ false };
static std::thread g_brainThread;
static std::atomic<bool> g_brainThreadDone{ true };
static std::atomic<bool> g_brainCancel{ false };
static std::mutex g_brainMtx;
static std::vector<std::pair<std::string, std::string>> g_chatMsgs;
static std::string g_chatInputBuf;
static std::string g_brainStatus;
static std::atomic<bool> g_brainThinking{ false };
static double g_brainThinkStart = 0.0;
static std::string g_brainDraft;
static std::mutex g_draftMtx;
static void SetDraft(const std::string& s) { std::lock_guard<std::mutex> lk(g_draftMtx);g_brainDraft = s; }
static std::string GetDraft() { std::lock_guard<std::mutex> lk(g_draftMtx);return g_brainDraft; }
static bool g_brainPendingStart = false;
static SdProvisioner g_brainProv;
static std::string g_ollamaModel = "gemma3:1b";
static std::atomic<bool> g_ollamaOk{ false };
static bool g_useOllama = true;
static bool OllamaPing();
static bool OllamaHasModel(const std::string& model);
static bool OllamaPullModel(const std::string& model, std::string& errOut, std::function<void(int)> onPct = nullptr);
static bool OllamaChat(const std::string&, const std::string&, const std::string&, const std::string&, std::string&, std::string&, const std::atomic<bool>*, std::function<void(const std::string&)>);
static std::wstring FindOllamaExe();
static void OllamaEnsureService();
static void OllamaAutoSetup();
static void OllamaSetupOneClick();
static std::string g_coachModel = "gemma3:4b";
static std::atomic<bool> g_coachModelReady{ false };
static std::string RgbToPng(const uint8_t* rgb, int w, int h);
static bool RobloxWindowBgra(std::vector<uint8_t>& out, int& w, int& h);
static std::string CaptureGameB64();
static void RemapKeys(std::string& s) {
	size_t p = s.find('(');
	while (p != std::string::npos && p + 2 < s.size() && s[p + 2] == ')') {
		char c = s[p + 1];
		if (c >= 'a' && c <= 'z')c = (char)(c - 'a' + 'A');
		char tmp[2] = { c,0 };
		const char* mapped = KbKey(tmp);
		if (mapped && mapped[0] && mapped[0] != c)s[p + 1] = mapped[0];
		p = s.find('(', p + 3);
	}
}
static void CoachAiWorkerBody(int targetId, float dx, float dy, int w, int h) {
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	std::string oModel, gameN;
	{
		std::lock_guard<std::mutex> lk(g_cfgStrMtx);
		oModel = g_ollamaModel;
		gameN = g_gameName;
	}
	std::string sys = "You are a Roblox coach overlay. Be extremely brief,one sentence,imperative.";
	if (!gameN.empty())sys += " Game: " + gameN + ".";
	sys += " Look at the attached screenshot and judge where the enemy is relative to your view.";
	char userTxt[160];
	if (targetId < 0) {
		sprintf_s(userTxt, "No targets visible. Give ONE short tactical instruction to keep moving and stay aware. Format: DIRECTION(KEY)example: Ahead(W)");
	}
	else {
		const char* dirX = std::abs(dx) < (float)w * 0.1f ? "centered" : (dx > 0 ? "to your RIGHT" : "to your LEFT");
		const char* dirY = std::abs(dy) < (float)h * 0.1f ? "" : (dy > 0 ? " and slightly BEHIND you" : " and slightly AHEAD of you");
		sprintf_s(userTxt, "A player is %s%s. Reply with ONE short instruction using exactly this format: DIRECTION(KEY)example: Right(D)or Left(A)or Ahead(W)or Behind(S)or Engage(LMB)", dirX, dirY);
	}
	std::string out, err;
	bool ok = false;
	try {
		if (g_useOllama && OllamaPing()) {
			std::string model = oModel;
			bool visionModel = false;
			if (!g_coachModelReady.load() && OllamaHasModel(g_coachModel)) {
				g_coachModelReady.store(true);
				visionModel = true;
			}
			else if (g_coachModelReady.load()) {
				visionModel = true;
			}
			if (visionModel)model = g_coachModel;
			std::string img;
			if (visionModel)img = CaptureGameB64();
			if (OllamaHasModel(model)) {
				ok = OllamaChat(model, sys, userTxt, img, out, err, nullptr, nullptr);
				if (!ok && !img.empty()) {
					std::string err2;
					ok = OllamaChat(oModel, sys, userTxt, "", out, err2, nullptr, nullptr);
				}
			}
		}
		else {
			std::lock_guard<std::mutex> onk(g_onnxMtx);
			if (!g_brain.IsReady()) {
				std::string e2;
				if (!g_brain.Init(e2)) { g_coachAiBusy.store(false);return; }
			}
			ok = g_brain.Generate(sys, userTxt, 30, out, err, nullptr, nullptr);
		}
	}
	catch (...) { ok = false; }
	std::string finalTxt;
	if (ok && !out.empty() && g_coachTargetId == targetId) {
		finalTxt = out;
		if (finalTxt.size() > 160)finalTxt.resize(160);
	}
	std::string up = finalTxt;
	std::transform(up.begin(), up.end(), up.begin(), [](unsigned char c) {return (char)std::toupper(c);});
	bool sane = up.find("RIGHT") != std::string::npos || up.find("LEFT") != std::string::npos ||
		up.find("AHEAD") != std::string::npos || up.find("BEHIND") != std::string::npos ||
		up.find("ENGAGE") != std::string::npos || up.find("LMB") != std::string::npos ||
		up.find("RMB") != std::string::npos;
	if (!sane) {
		const char* key = "W";
		if (dx > (float)w * 0.1f)key = "D";
		else if (dx < -(float)w * 0.1f)key = "A";
		else if (dy > (float)h * 0.1f)key = "S";
		const char* nm = key[0] == 'D' ? "Right" : (key[0] == 'A' ? "Left" : (key[0] == 'S' ? "Behind" : "Ahead"));
		char fb[64];
		sprintf_s(fb, "%s(%c)", nm, key[0]);
		finalTxt = fb;
	}
	RemapKeys(finalTxt);
	if (g_coachTargetId == targetId && g_coachOn) {
		std::lock_guard<std::mutex> lk(g_coachTextMtx);
		g_coachAiText = finalTxt;
	}
	g_coachAiBusy.store(false);
}
struct CoachAskCtx { int targetId;float dx;float dy;int w;int h; };
static bool SehCoachAsk(void* p) {
	CoachAskCtx* c = (CoachAskCtx*)p;
	CoachAiWorkerBody(c->targetId, c->dx, c->dy, c->w, c->h);
	return true;
}
static void CoachAiWorker(int targetId, float dx, float dy, int w, int h) {
	CoachAskCtx ctx{ targetId,dx,dy,w,h };
	std::string sehErr;
	SehCall(SehCoachAsk, &ctx, sehErr);
	g_coachAiBusy.store(false);
}
static std::thread g_coachThread;
static std::atomic<bool> g_coachThreadDone{ true };
static void CoachAiAsk(int targetId, float dx, float dy, int w, int h) {
	if (!g_coachOn || g_coachAiBusy.load() || g_brainRun.load())return;
	if (g_coachThread.joinable()) {
		if (g_coachThreadDone.load())g_coachThread.join();
		else return;
	}
	g_coachThreadDone.store(false);
	g_coachAiBusy.store(true);
	g_coachThread = std::thread([targetId, dx, dy, w, h] {
		CoachAiWorker(targetId, dx, dy, w, h);
		g_coachThreadDone.store(true);
		});
}
static std::string B64Encode(const uint8_t* data, size_t n) {
	static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string out;
	out.reserve((n + 2) / 3 * 4);
	for (size_t i = 0;i < n;i += 3) {
		uint32_t v = (uint32_t)data[i] << 16;
		if (i + 1 < n)v |= (uint32_t)data[i + 1] << 8;
		if (i + 2 < n)v |= (uint32_t)data[i + 2];
		out += tbl[(v >> 18) & 63];
		out += tbl[(v >> 12) & 63];
		out += (i + 1 < n) ? tbl[(v >> 6) & 63] : '=';
		out += (i + 2 < n) ? tbl[v & 63] : '=';
	}
	return out;
}
static uint32_t Crc32Table[256];
static void Crc32Init() {
	for (uint32_t i = 0;i < 256;++i) {
		uint32_t c = i;
		for (int k = 0;k < 8;++k)c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
		Crc32Table[i] = c;
	}
}
static uint32_t Crc32(const uint8_t* d, size_t n) {
	static bool init = false;
	if (!init) { Crc32Init();init = true; }
	uint32_t c = 0xFFFFFFFFu;
	for (size_t i = 0;i < n;++i)c = Crc32Table[(c ^ d[i]) & 0xFF] ^ (c >> 8);
	return c ^ 0xFFFFFFFFu;
}
static void PngChunk(std::string& out, const char* type, const uint8_t* data, size_t n) {
	uint32_t len = (uint32_t)n;
	out.push_back((char)(len >> 24));out.push_back((char)(len >> 16));out.push_back((char)(len >> 8));out.push_back((char)len);
	out.append(type, 4);
	out.append((const char*)data, n);
	{
		uint8_t tmp[8];
		memcpy(tmp, type, 4);
		uint32_t c2 = 0xFFFFFFFFu;
		for (size_t i = 0;i < 4;++i)c2 = Crc32Table[(c2 ^ tmp[i]) & 0xFF] ^ (c2 >> 8);
		for (size_t i = 0;i < n;++i)c2 = Crc32Table[(c2 ^ data[i]) & 0xFF] ^ (c2 >> 8);
		c2 ^= 0xFFFFFFFFu;
		out.push_back((char)(c2 >> 24));out.push_back((char)(c2 >> 16));out.push_back((char)(c2 >> 8));out.push_back((char)c2);
	}
}
static std::string RgbToPng(const uint8_t* rgb, int w, int h) {
	std::string out;
	out.append("\x89PNG\r\n\x1a\n", 8);
	{
		uint8_t ihdr[13];
		ihdr[0] = (uint8_t)(w >> 24);ihdr[1] = (uint8_t)(w >> 16);ihdr[2] = (uint8_t)(w >> 8);ihdr[3] = (uint8_t)w;
		ihdr[4] = (uint8_t)(h >> 24);ihdr[5] = (uint8_t)(h >> 16);ihdr[6] = (uint8_t)(h >> 8);ihdr[7] = (uint8_t)h;
		ihdr[8] = 8;ihdr[9] = 2;ihdr[10] = 0;ihdr[11] = 0;ihdr[12] = 0;
		PngChunk(out, "IHDR", ihdr, 13);
	}
	{
		std::string raw;
		raw.reserve((size_t)w * 3 * h + h);
		for (int y = 0;y < h;++y) {
			raw.push_back(0);
			raw.append((const char*)(rgb + (size_t)y * w * 3), (size_t)w * 3);
		}
		std::string zlib;
		zlib.push_back(0x78);zlib.push_back(0x01);
		size_t pos = 0;
		while (pos < raw.size()) {
			size_t chunk = std::min<size_t>(65535, raw.size() - pos);
			zlib.push_back((char)(chunk & 0xFF));
			zlib.push_back((char)((chunk >> 8) & 0xFF));
			zlib.push_back(0x00);
			zlib.append(raw, pos, chunk);
			pos += chunk;
		}
		uint32_t adler = 1;
		for (size_t i = 0;i < raw.size();++i) {
			adler = (adler + (uint32_t)(uint8_t)raw[i]) % 65521;
			adler = ((adler + (((uint32_t)(uint8_t)raw[i]) << 8)) % 65521);
		}
		{
			uint32_t a = 1, b = 0;
			for (size_t i = 0;i < raw.size();++i) { a = (a + (uint8_t)raw[i]) % 65521;b = (b + a) % 65521; }
			adler = (b << 16) | a;
		}
		zlib.push_back((char)((adler >> 24) & 0xFF));zlib.push_back((char)((adler >> 16) & 0xFF));
		zlib.push_back((char)((adler >> 8) & 0xFF));zlib.push_back((char)(adler & 0xFF));
		PngChunk(out, "IDAT", (const uint8_t*)zlib.data(), zlib.size());
	}
	PngChunk(out, "IEND", nullptr, 0);
	return out;
}
static std::mutex g_capShotMtx;
static bool RobloxWindowBgra(std::vector<uint8_t>& out, int& w, int& h) {
	std::lock_guard<std::mutex> ck(g_capShotMtx);
	out.clear();w = 0;h = 0;
	if (!g_robloxHwnd)return false;
	RECT rc{};
	if (!GetWindowRect(g_robloxHwnd, &rc))return false;
	int W = rc.right - rc.left, H = rc.bottom - rc.top;
	if (W < 64 || H < 64)return false;
	HDC dc = GetDC(nullptr);
	HDC mdc = CreateCompatibleDC(dc);
	BITMAPINFO bi{};
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = W;
	bi.bmiHeader.biHeight = -H;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	void* bits = nullptr;
	HBITMAP bmp = CreateDIBSection(mdc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
	if (!bmp || !bits) {
		if (bmp)DeleteObject(bmp);
		DeleteDC(mdc);ReleaseDC(nullptr, dc);
		return false;
	}
	HGDIOBJ old = SelectObject(mdc, bmp);
	BOOL ok = PrintWindow(g_robloxHwnd, mdc, 2);
	SelectObject(mdc, old);
	DeleteObject(bmp);
	DeleteDC(mdc);
	ReleaseDC(nullptr, dc);
	if (!ok)return false;
	out.resize((size_t)W * H * 4);
	memcpy(out.data(), bits, out.size());
	w = W;h = H;
	return true;
}
static std::string LastFramePngB64() {
	std::vector<uint8_t> frame;
	int fw = 0, fh = 0;
	{
		std::lock_guard<std::mutex> lk(g_radarFrameMtx);
		if (g_radarFrameBytes.empty())return "";
		frame = g_radarFrameBytes;
		fw = g_radarFrameW;fh = g_radarFrameH;
	}
	if (fw <= 0 || fh <= 0)return "";
	int tw = 160, th = std::max(1, fh * tw / fw);
	std::vector<uint8_t> rgb((size_t)tw * th * 3);
	for (int y = 0;y < th;++y) {
		int sy = std::min(fh - 1, y * fh / th);
		for (int x = 0;x < tw;++x) {
			int sx = std::min(fw - 1, x * fw / tw);
			const uint8_t* p = &frame[((size_t)sy * fw + sx) * 4];
			rgb[((size_t)y * tw + x) * 3 + 0] = p[2];
			rgb[((size_t)y * tw + x) * 3 + 1] = p[1];
			rgb[((size_t)y * tw + x) * 3 + 2] = p[0];
		}
	}
	std::string png = RgbToPng(rgb.data(), tw, th);
	return B64Encode((const uint8_t*)png.data(), png.size());
}
static std::string CaptureGameB64() {
	std::vector<uint8_t> vf;
	int vw = 0, vh = 0;
	if (!RobloxWindowBgra(vf, vw, vh) || vw <= 0 || vh <= 0)return "";
	int tw = 320, th = std::max(1, vh * tw / vw);
	std::vector<uint8_t> rgb((size_t)tw * th * 3);
	for (int y = 0;y < th;++y) {
		int sy = std::min(vh - 1, y * vh / th);
		for (int x = 0;x < tw;++x) {
			int sx = std::min(vw - 1, x * vw / tw);
			const uint8_t* p = &vf[((size_t)sy * vw + sx) * 4];
			rgb[((size_t)y * tw + x) * 3 + 0] = p[2];
			rgb[((size_t)y * tw + x) * 3 + 1] = p[1];
			rgb[((size_t)y * tw + x) * 3 + 2] = p[0];
		}
	}
	std::string png = RgbToPng(rgb.data(), tw, th);
	if (png.empty())return "";
	return B64Encode((const uint8_t*)png.data(), png.size());
}
static void OllamaTimeouts(HINTERNET ses) {
	try { WinHttpSetTimeouts(ses, 4000, 4000, 8000, 15000); }
	catch (...) {}
}
static bool OllamaPing() {
	HINTERNET ses = WinHttpOpen(L"StudReshader", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!ses)return false;
	OllamaTimeouts(ses);
	HINTERNET con = WinHttpConnect(ses, L"127.0.0.1", 11434, 0);
	if (!con) { WinHttpCloseHandle(ses);return false; }
	HINTERNET req = WinHttpOpenRequest(con, L"GET", L"/api/tags", nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);return false; }
	bool ok = false;
	if (WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
		if (WinHttpReceiveResponse(req, nullptr))ok = true;
	WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
	return ok;
}
static bool OllamaHttp(const std::wstring& method, const std::wstring& path, const std::string& body,
	std::string& resp, std::string& errOut, const std::atomic<bool>* cancel = nullptr,
	std::function<void(const std::string&)> onChunk = nullptr) {
	resp.clear();
	HINTERNET ses = WinHttpOpen(L"StudReshader", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!ses) { errOut = "winhttp open failed";return false; }
	OllamaTimeouts(ses);
	HINTERNET con = WinHttpConnect(ses, L"127.0.0.1", 11434, 0);
	if (!con) { WinHttpCloseHandle(ses);errOut = "ollama not running";return false; }
	HINTERNET req = WinHttpOpenRequest(con, method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);errOut = "req failed";return false; }
	bool ok = false;
	if (body.empty()) {
		ok = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) != 0;
	}
	else {
		std::wstring headers = L"Content-Type: application/json\r\n";
		ok = WinHttpSendRequest(req, headers.c_str(), (DWORD)-1L, (LPVOID)body.data(), (DWORD)body.size(), (DWORD)body.size(), 0) != 0;
	}
	if (ok)ok = WinHttpReceiveResponse(req, nullptr) != 0;
	if (ok) {
		char buf[2048];
		DWORD got = 0;
		while (WinHttpReadData(req, buf, sizeof(buf), &got) && got > 0) {
			if (cancel && cancel->load())break;
			resp.append(buf, got);
			if (onChunk)onChunk(std::string(buf, got));
		}
		ok = true;
	}
	else {
		errOut = "http failed " + std::to_string(GetLastError());
	}
	WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
	return ok;
}
static bool OllamaHasModel(const std::string& model) {
	std::string resp, err;
	if (!OllamaHttp(L"GET", L"/api/tags", "", resp, err))return false;
	return resp.find("\"" + model + "\"") != std::string::npos;
}
static std::atomic<bool> g_ollamaBusy{ false };
static std::atomic<double> g_ollamaPct{ 0.0 };
static std::atomic<bool> g_ollamaDlCli{ false };
static std::mutex g_statusMtx;
static void SetStatus(const std::string& s) { std::lock_guard<std::mutex> lk(g_statusMtx);g_brainStatus = s; }
static std::string GetStatus() { std::lock_guard<std::mutex> lk(g_statusMtx);return g_brainStatus; }
static std::string g_ollamaStep;
static void SetStep(const std::string& s) { std::lock_guard<std::mutex> lk(g_statusMtx);g_ollamaStep = s; }
static std::string GetStep() { std::lock_guard<std::mutex> lk(g_statusMtx);return g_ollamaStep; }
static bool OllamaPullModel(const std::string& model, std::string& errOut,
	std::function<void(int)> onPct) {
	std::string body = "{\"name\":\"" + model + "\",\"stream\":true}";
	g_ollamaPulling.store(true);
	g_ollamaPct.store(0.0);
	double total = 0.0, completed = 0.0;
	std::string pending;
	bool errFlag = false;
	auto onChunk = [&](const std::string& chunk) {
		pending += chunk;
		size_t nl;
		while ((nl = pending.find('\n')) != std::string::npos) {
			std::string line = pending.substr(0, nl);
			pending.erase(0, nl + 1);
			if (line.empty())continue;
			if (line.find("\"error\"") != std::string::npos) { errFlag = true;continue; }
			auto findNum = [&](const char* key)-> double {
				std::string k = "\"" + std::string(key) + "\":";
				size_t ks = line.find(k);
				if (ks == std::string::npos)return -1.0;
				size_t vs = ks + k.size();
				double v = 0.0;
				bool neg = false;
				size_t p = vs;
				while (p < line.size() && (line[p] == ' ' || line[p] == '\t'))++p;
				if (p < line.size() && line[p] == '-') { neg = true;++p; }
				bool any = false;
				while (p < line.size() && line[p] >= '0' && line[p] <= '9') { v = v * 10.0 + (line[p] - '0');++p;any = true; }
				if (p < line.size() && line[p] == '.') {
					++p;
					double frac = 0.1;
					while (p < line.size() && line[p] >= '0' && line[p] <= '9') { v += (line[p] - '0') * frac;frac *= 0.1;++p;any = true; }
				}
				if (!any)return -1.0;
				return neg ? -v : v;
				};
			double t = findNum("total"), c = findNum("completed");
			if (t > 0)total = t;
			if (c >= 0)completed = c;
			if (total > 0 && completed >= 0) {
				double pct = std::clamp(completed / total * 100.0, 0.0, 100.0);
				g_ollamaPct.store(pct);
				if (onPct)onPct((int)pct);
			}
		}
		};
	std::string resp, err;
	bool ok = OllamaHttp(L"POST", L"/api/pull", body, resp, err, nullptr, onChunk);
	errOut = err;
	if (errFlag) { errOut = "model not found on Ollama registry";ok = false; }
	g_ollamaPulling.store(false);
	g_ollamaPct.store(ok ? 100.0 : 0.0);
	return ok;
}
static bool OllamaChat(const std::string& model, const std::string& system, const std::string& user,
	const std::string& imgB64, std::string& outText, std::string& errOut,
	const std::atomic<bool>* cancel, std::function<void(const std::string&)> onDraft) {
	outText.clear();
	auto esc = [](const std::string& s) {
		std::string o;
		o.reserve(s.size() + 8);
		for (char c : s) {
			if (c == '\\' || c == '"') { o.push_back('\\');o.push_back(c); }
			else if (c == '\n')o += "\\n";
			else if (c == '\r')o += "\\r";
			else if (c == '\t')o += "\\t";
			else o.push_back(c);
		}
		return o;
		};
	std::string body = "{\"model\":\"" + esc(model) + "\",\"stream\":true,\"messages\":[{\"role\":\"system\",\"content\":\"" + esc(system) + "\"},";
	body += "{\"role\":\"user\",\"content\":\"" + esc(user) + "\"";
	if (!imgB64.empty())body += ",\"images\":[\"" + imgB64 + "\"]";
	body += "}]}";
	HINTERNET ses = WinHttpOpen(L"StudReshader", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!ses) { errOut = "winhttp open failed";return false; }
	OllamaTimeouts(ses);
	HINTERNET con = WinHttpConnect(ses, L"127.0.0.1", 11434, 0);
	if (!con) { WinHttpCloseHandle(ses);errOut = "ollama not running";return false; }
	HINTERNET req = WinHttpOpenRequest(con, L"POST", L"/api/chat", nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!req) { WinHttpCloseHandle(con);WinHttpCloseHandle(ses);errOut = "req failed";return false; }
	std::wstring headers = L"Content-Type: application/json\r\n";
	bool ok = false;
	if (WinHttpSendRequest(req, headers.c_str(), (DWORD)-1L, (LPVOID)body.data(), (DWORD)body.size(), (DWORD)body.size(), 0)) {
		if (WinHttpReceiveResponse(req, nullptr)) {
			std::string accum;
			char buf[2048];
			DWORD got = 0;
			while (WinHttpReadData(req, buf, sizeof(buf), &got) && got > 0) {
				if (cancel && cancel->load())break;
				accum.append(buf, got);
				size_t pos = 0;
				size_t nl;
				while ((nl = accum.find('\n', pos)) != std::string::npos) {
					std::string line = accum.substr(pos, nl - pos);
					pos = nl + 1;
					if (line.find("\"content\"") != std::string::npos) {
						size_t cs = line.find("\"content\":\"");
						if (cs != std::string::npos) {
							cs += 11;
							size_t ce = line.find('\"', cs);
							if (ce != std::string::npos) {
								std::string piece = line.substr(cs, ce - cs);
								for (size_t k = 0;k < piece.size();++k) {
									if (piece[k] == '\\' && k + 1 < piece.size()) {
										char nx = piece[k + 1];
										if (nx == 'n') { outText.push_back('\n');++k; }
										else if (nx == 't') { outText.push_back('\t');++k; }
										else if (nx == '"') { outText.push_back('"');++k; }
										else if (nx == '\\') { outText.push_back('\\');++k; }
										else { outText.push_back(piece[k]); }
									}
									else {
										outText.push_back(piece[k]);
									}
								}
								if (onDraft)onDraft(outText);
							}
						}
					}
				}
				accum.erase(0, pos);
			}
			ok = true;
		}
	}
	else {
		DWORD ec = GetLastError();
		errOut = "send failed " + std::to_string(ec);
	}
	WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
	if (cancel && cancel->load()) { errOut = "cancelled";return false; }
	if (ok && outText.empty() && errOut.empty())errOut = "empty reply from assistant";
	return ok && !outText.empty();
}
static std::wstring FindOllamaExe() {
	std::vector<wchar_t> sys(1024, 0);
	if (GetEnvironmentVariableW(L"OLLAMA_HOME", sys.data(), (DWORD)sys.size()) && sys[0]) {
		std::wstring p = sys.data();
		if (GetFileAttributesW((p + L"\\ollama.exe").c_str()) != INVALID_FILE_ATTRIBUTES)return p + L"\\ollama.exe";
	}
	std::vector<wchar_t> path(8192, 0);
	if (GetEnvironmentVariableW(L"PATH", path.data(), (DWORD)path.size()) && path[0]) {
		std::wstring ps(path.data());
		size_t st = 0;
		while (st <= ps.size()) {
			size_t se = ps.find(L';', st);
			if (se == std::wstring::npos)se = ps.size();
			std::wstring d = ps.substr(st, se - st);
			if (GetFileAttributesW((d + L"\\ollama.exe").c_str()) != INVALID_FILE_ATTRIBUTES)return d + L"\\ollama.exe";
			st = se + 1;
		}
	}
	std::wstring appData;
	if (GetEnvironmentVariableW(L"LOCALAPPDATA", sys.data(), (DWORD)sys.size()) && sys[0]) {
		appData = sys.data();
		std::wstring p = appData + L"\\Programs\\Ollama\\ollama.exe";
		if (GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES)return p;
	}
	return L"";
}
static void OllamaInstall() {
	std::error_code ec;
	std::string oModel;
	{ std::lock_guard<std::mutex> lk(g_cfgStrMtx);oModel = g_ollamaModel; }
	std::wstring existing = FindOllamaExe();
	if (!existing.empty()) {
		SetStatus("Assistant found | starting it...");
		STARTUPINFOW si{};PROCESS_INFORMATION pi{};
		si.cb = sizeof(si);
		std::wstring cmd = L"\"" + existing + L"\" serve";
		std::vector<wchar_t> cbuf(cmd.begin(), cmd.end());cbuf.push_back(0);
		if (CreateProcessW(nullptr, cbuf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
			CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
		}
		for (int w = 0;w < 60;++w) {
			Sleep(500);
			if (OllamaPing())break;
		}
		if (OllamaPing()) {
			g_ollamaOk.store(true);
			SetStatus("Assistant running | " + oModel + " ready");
			return;
		}
		SetStatus("Assistant found but not reachable | start it manually");
		return;
	}
	std::filesystem::create_directories(L"models\\ollama", ec);
	std::wstring exePath;
	wchar_t buf[MAX_PATH] = {};
	if (GetModuleFileNameW(nullptr, buf, MAX_PATH))exePath = buf;
	std::wstring dir = L"";
	size_t sl = exePath.find_last_of(L"\\/");
	if (sl != std::wstring::npos)dir = exePath.substr(0, sl + 1);
	std::wstring oDir = dir + L"models\\ollama\\";
	std::wstring zip = oDir + L"ollama.zip";
	std::wstring oexe = oDir + L"ollama.exe";
	std::atomic<double> prog{ 0.0 };
	std::string err;
	g_ollamaDlCli.store(true);
	SetStep("Downloading assistant(~1.4 GB)...");
	SetStatus(GetStep());
	g_ollamaPct.store(0.0);
	std::thread([&] {
		while (g_ollamaDlCli.load()) {
			g_ollamaPct.store(prog.load() * 100.0);
			Sleep(200);
		}
		}).detach();
	if (!DownloadUrlToFile(L"https://ollama.com/download/ollama-windows-amd64.zip", zip, &prog, &err)) {
		g_ollamaDlCli.store(false);
		SetStatus("Ollama download failed: " + err + " | install manually from ollama.com");
		return;
	}
	g_ollamaDlCli.store(false);
	SetStatus("Extracting...");
	if (!ExtractZipToDirectory(zip, oDir)) {
		SetStatus("Ollama extract failed");
		DeleteFileW(zip.c_str());
		return;
	}
	DeleteFileW(zip.c_str());
	if (GetFileAttributesW(oexe.c_str()) == INVALID_FILE_ATTRIBUTES) {
		SetStatus("Ollama CLI missing after extract");
		return;
	}
	SetStatus("Starting assistant...");
	STARTUPINFOW si{};PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);
	std::wstring cmd = L"\"" + oexe + L"\" serve";
	std::vector<wchar_t> cbuf(cmd.begin(), cmd.end());cbuf.push_back(0);
	if (CreateProcessW(nullptr, cbuf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
		CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
	}
	for (int w = 0;w < 60;++w) {
		Sleep(500);
		if (OllamaPing())break;
	}
	if (!OllamaPing()) {
		SetStatus("Started but not reachable | start it manually");
		return;
	}
	SetStatus("Checking model " + oModel + "...");
	if (!OllamaHasModel(oModel)) {
		SetStatus("Downloading model " + oModel + "(first time,~3 GB)...");
		std::string perr;
		if (!OllamaPullModel(oModel, perr)) {
			SetStatus("Model download failed: " + perr);
			return;
		}
	}
	g_ollamaOk.store(true);
	SetStatus("Ready | " + oModel + " loaded");
}
static void OllamaEnsureService() {
	if (OllamaPing())return;
	std::wstring exe = FindOllamaExe();
	if (exe.empty()) {
		wchar_t buf[MAX_PATH] = {};
		if (GetModuleFileNameW(nullptr, buf, MAX_PATH)) {
			std::wstring dir(buf);
			size_t sl = dir.find_last_of(L"\\/");
			std::wstring mine = (sl != std::wstring::npos ? dir.substr(0, sl + 1) : L"") + L"models\\ollama\\ollama.exe";
			if (GetFileAttributesW(mine.c_str()) != INVALID_FILE_ATTRIBUTES)exe = mine;
		}
	}
	if (exe.empty())return;
	STARTUPINFOW si{};PROCESS_INFORMATION pi{};
	si.cb = sizeof(si);
	std::wstring cmd = L"\"" + exe + L"\" serve";
	std::vector<wchar_t> cbuf(cmd.begin(), cmd.end());cbuf.push_back(0);
	if (CreateProcessW(nullptr, cbuf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
		CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
	}
	for (int w = 0;w < 40;++w) {
		Sleep(500);
		if (OllamaPing())break;
	}
}
static void OllamaAutoSetup() {
	if (!g_useOllama || g_ollamaBusy.load())return;
	g_ollamaBusy.store(true);
	SetStatus("Checking...");
	std::thread([] {
		std::string oModel;
		{ std::lock_guard<std::mutex> lk(g_cfgStrMtx);oModel = g_ollamaModel; }
		OllamaEnsureService();
		if (OllamaPing()) {
			if (OllamaHasModel(oModel)) {
				g_ollamaOk.store(true);
				SetStatus("Ready | " + oModel);
			}
			else {
				g_ollamaOk.store(false);
				SetStatus("Assistant found but model missing | click Install Assistant to download it");
			}
		}
		else {
			g_ollamaOk.store(false);
			SetStatus("Assistant not found | click Install Assistant to set it up");
		}
		g_ollamaBusy.store(false);
		}).detach();
}
static void OllamaSetupOneClick() {
	if (g_ollamaBusy.load()) {
		SetStatus("Already installing | please wait");
		return;
	}
	g_ollamaBusy.store(true);
	SetStatus("Starting...");
	std::thread([] {
		std::string oModel;
		{ std::lock_guard<std::mutex> lk(g_cfgStrMtx);oModel = g_ollamaModel; }
		OllamaEnsureService();
		if (OllamaPing()) {
			if (OllamaHasModel(oModel)) {
				g_ollamaOk.store(true);
				SetStatus("Ready | " + oModel + " loaded");
				g_ollamaBusy.store(false);
				return;
			}
			SetStep("Downloading model " + oModel + "(~3 GB)...");
			SetStatus(GetStep());
			std::string perr;
			if (OllamaPullModel(oModel, perr) && OllamaHasModel(oModel)) {
				g_ollamaOk.store(true);
				SetStatus("Ready | " + oModel + " loaded");
			}
			else {
				g_ollamaOk.store(false);
				SetStatus("Model download failed: " + perr);
			}
			g_ollamaBusy.store(false);
			return;
		}
		OllamaInstall();
		if (OllamaPing() && !OllamaHasModel(oModel)) {
			SetStep("Downloading model " + oModel + "(~3 GB)...");
			SetStatus(GetStep());
			std::string perr;
			OllamaPullModel(oModel, perr);
		}
		if (OllamaPing() && OllamaHasModel(oModel)) {
			g_ollamaOk.store(true);
			SetStatus("Ready | " + oModel + " loaded");
		}
		g_ollamaBusy.store(false);
		}).detach();
}
static void BrainAddMsg(const std::string& role, const std::string& text) {
	std::lock_guard<std::mutex> lk(g_brainMtx);
	g_chatMsgs.emplace_back(role, text);
	if (g_chatMsgs.size() > 60)g_chatMsgs.erase(g_chatMsgs.begin());
}
static std::string BrainSystemPrompt() {
	std::string sys = "You are an assistant inside a Roblox overlay app. The user is playing Roblox right now. ";
	std::string gn;
	{ std::lock_guard<std::mutex> lk(g_cfgStrMtx);gn = g_gameName; }
	if (!gn.empty())sys += "The game is " + gn + ". ";
	int n = 0;
	{
		std::lock_guard<std::mutex> lk(g_radarMtx);
		n = (int)g_radarDets.size();
	}
	sys += "The player detector currently sees " + std::to_string(n) + " people on screen. ";
	{
		std::lock_guard<std::mutex> lk2(g_radarMtx);
		if (!g_radarDets.empty() && g_radarFW > 0 && g_radarFH > 0) {
			sys += "Player positions(x% of width,y% of height from top):";
			int cnt = 0;
			for (auto& d : g_radarDets) {
				if (cnt >= 6)break;
				char pb[64];
				sprintf_s(pb, "(%d%%,%d%%)", (int)(d.cx / (float)g_radarFW * 100.0f), (int)(d.cy / (float)g_radarFH * 100.0f));
				sys += pb;
				++cnt;
			}
			sys += ". ";
		}
	}
	sys += "Answer in 1-3 short sentences. If the user asks you to guide them,end your reply with[COACH_ON]. ";
	sys += "If they ask to stop guidance,end with[COACH_OFF]. Do not mention these tags.";
	return sys;
}
static void BrainWorkerBody();
static bool SehBrainWorkerBody(void* p);
static void BrainWorker() {
	std::string sehErr;
	SehCall(SehBrainWorkerBody, nullptr, sehErr);
	g_brainThreadDone.store(true);
}
static bool SehBrainWorkerBody(void*) {
	BrainWorkerBody();
	return true;
}
static void BrainWorkerBody() {
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	std::string lastUser;
	{
		std::lock_guard<std::mutex> lk(g_brainMtx);
		if (!g_chatMsgs.empty() && g_chatMsgs.back().first == "user")
			lastUser = g_chatMsgs.back().second;
	}
	std::string out, err;
	std::atomic<int> nt{ 0 };
	std::string oModel;
	{
		std::lock_guard<std::mutex> lk(g_cfgStrMtx);
		oModel = g_ollamaModel;
	}
	SetDraft("");
	g_brainThinking.store(true);
	g_brainThinkStart = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
	bool usedOllama = false;
	bool ok = false;
	if (g_ollamaBusy.load()) {
		g_brainThinking.store(false);
		BrainAddMsg("assistant", "(the assistant is still installing,give it a minute)");
		SetStatus("Still installing | please wait");
		g_brainRun.store(false);
		g_brainThreadDone.store(true);
		return;
	}
	if (lastUser.empty()) {
		g_brainThinking.store(false);
		SetStatus("Ready");
		g_brainRun.store(false);
		g_brainThreadDone.store(true);
		return;
	}
	std::string lower = lastUser;
	std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {return (char)std::tolower(c);});
	bool wantsVision = lower.find("look") != std::string::npos || lower.find("see") != std::string::npos ||
		lower.find("screen") != std::string::npos || lower.find("vision") != std::string::npos ||
		lower.find("watch") != std::string::npos || lower.find("what do you see") != std::string::npos ||
		lower.find("player") != std::string::npos || lower.find("enemy") != std::string::npos ||
		lower.find("where") != std::string::npos || lower.find("how many") != std::string::npos;
	if (g_useOllama && OllamaPing()) {
		g_ollamaOk.store(true);
		SetStatus("Assistant | " + oModel);
		std::string img = "";
		if (wantsVision) {
			std::vector<uint8_t> vf;
			int vw = 0, vh = 0;
			if (RobloxWindowBgra(vf, vw, vh) && vw > 0 && vh > 0) {
				int tw = 160, th = std::max(1, vh * tw / vw);
				std::vector<uint8_t> rgb((size_t)tw * th * 3);
				for (int y = 0;y < th;++y) {
					int sy = std::min(vh - 1, y * vh / th);
					for (int x = 0;x < tw;++x) {
						int sx = std::min(vw - 1, x * vw / tw);
						const uint8_t* p = &vf[((size_t)sy * vw + sx) * 4];
						rgb[((size_t)y * tw + x) * 3 + 0] = p[2];
						rgb[((size_t)y * tw + x) * 3 + 1] = p[1];
						rgb[((size_t)y * tw + x) * 3 + 2] = p[0];
					}
				}
				std::string png = RgbToPng(rgb.data(), tw, th);
				img = B64Encode((const uint8_t*)png.data(), png.size());
			}
		}
		usedOllama = true;
		if (!OllamaHasModel(oModel)) {
			SetStatus("Model " + oModel + " missing | downloading it now...");
			std::string perr;
			if (OllamaPullModel(oModel, perr) && OllamaHasModel(oModel)) {
				SetStatus("Model " + oModel + " ready");
			}
			else {
				g_ollamaOk.store(false);
				SetStatus("Model download failed: " + perr + " | using local fallback");
				usedOllama = false;
			}
		}
		if (usedOllama) {
			ok = OllamaChat(oModel, BrainSystemPrompt(), lastUser, img, out, err, &g_brainCancel,
				[](const std::string& part) {SetDraft(part);});
			if (!ok && !img.empty()) {
				std::string err2;
				img.clear();
				ok = OllamaChat(oModel, BrainSystemPrompt(), lastUser, "", out, err2, &g_brainCancel,
					[](const std::string& part) {SetDraft(part);});
			}
			if (!ok) {
				g_ollamaOk.store(false);
				SetStatus("Assistant error: " + (err.empty() ? "unknown" : err) + " | using local fallback");
				usedOllama = false;
			}
		}
	}
	else {
		g_ollamaOk.store(false);
		if (g_useOllama)SetStatus("Assistant not found | using local fallback");
	}
	if (!ok && !usedOllama) {
		if (!BrainFilesReady()) {
			SetStatus("Assistant unavailable | run Install Assistant first");
			BrainAddMsg("assistant", "(assistant not installed | click Install Assistant)");
			g_brainRun.store(false);
			g_brainThreadDone.store(true);
			return;
		}
		SetStatus("Loading local Qwen brain...");
		std::lock_guard<std::mutex> onk(g_onnxMtx);
		if (!g_brain.IsReady()) {
			std::string e2;
			if (!g_brain.Init(e2)) {
				SetStatus("Brain load failed: " + e2);
				g_brainRun.store(false);
				g_brainThreadDone.store(true);
				return;
			}
		}
		if (g_brain.IsCpu()) {
			if (g_epBroken.load())
				SetStatus("Brain on CPU | see models\\sr_gpu.log");
			else
				SetStatus("Brain on CPU | DirectML unavailable");
		}
		else {
			SetStatus("Brain on " + g_brain.GpuName());
		}
		ok = g_brain.Generate(BrainSystemPrompt(), lastUser, 64, out, err, &g_brainCancel, &nt,
			[](const std::string& part) {SetDraft(part);});
		{
			std::string t1 = out;
			while (!t1.empty() && (t1.back() == ' ' || t1.back() == '\n' || t1.back() == '\r'))t1.pop_back();
			if (!ok || t1.empty()) {
				std::string e3 = err;
				ok = g_brain.Generate(BrainSystemPrompt(), lastUser, 64, out, err, &g_brainCancel, &nt,
					[](const std::string& part) {SetDraft(part);});
				if (!ok && !e3.empty())err = e3;
			}
		}
	}
	g_brainThinking.store(false);
	if (!ok && out.empty()) {
		if (err.find("cancelled") == std::string::npos) {
			SetStatus("Assistant error: " + err);
			BrainAddMsg("assistant", "(error: " + err + ")");
		}
	}
	else {
		if (!out.empty()) {
			size_t rep = 0;
			for (size_t i = 1;i < out.size();++i)
				if (out[i] == out[i - 1])++rep;
			if (out.size() > 20 && (double)rep / (double)out.size() > 0.35) {
				BrainAddMsg("assistant", "(output unstable,try again)");
				SetStatus("Unstable output,try again");
				g_brainRun.store(false);
				g_brainThreadDone.store(true);
				return;
			}
		}
		bool coachOn = false, coachOff = false;
		if (out.find("[COACH_ON]") != std::string::npos)coachOn = true;
		if (out.find("[COACH_OFF]") != std::string::npos)coachOff = true;
		if (coachOn)out.erase(out.find("[COACH_ON]"), 10);
		if (coachOff)out.erase(out.find("[COACH_OFF]"), 11);
		{
			std::string low2 = lastUser;
			std::transform(low2.begin(), low2.end(), low2.begin(), [](unsigned char c) {return (char)std::tolower(c);});
			bool askGuide = low2.find("guide") != std::string::npos || low2.find("coach") != std::string::npos ||
				low2.find("enable") != std::string::npos || low2.find("help me play") != std::string::npos ||
				low2.find("tell me where") != std::string::npos || low2.find("what do i press") != std::string::npos;
			bool stopGuide = low2.find("stop") != std::string::npos || low2.find("disable") != std::string::npos ||
				low2.find("turn off") != std::string::npos || low2.find("shut up") != std::string::npos;
			if (askGuide && !stopGuide)coachOn = true;
			if (stopGuide)coachOff = true;
		}
		while (!out.empty() && (out.back() == ' ' || out.back() == '\n' || out.back() == '\r'))out.pop_back();
		if (out.empty()) {
			BrainAddMsg("assistant", "(no answer | try again)");
			SetStatus("Assistant gave no answer | try again");
			g_brainRun.store(false);
			g_brainThreadDone.store(true);
			return;
		}
		BrainAddMsg("assistant", out);
		if (coachOn && !g_coachOn) {
			g_coachOn = true;
			g_coachLastAskT = -10.0;
			{ std::lock_guard<std::mutex> ck(g_coachTextMtx);g_coachAiText.clear(); }
			SetStatus("Coach enabled | guiding you now");
		}
		else if (coachOff && g_coachOn) {
			g_coachOn = false;
			{ std::lock_guard<std::mutex> ck(g_coachTextMtx);g_coachAiText.clear(); }
			SetStatus("Coach disabled");
		}
		else {
			SetStatus("Ready");
		}
	}
	g_brainRun.store(false);
	g_brainThreadDone.store(true);
}
static void BrainAsk(const std::string& msg) {
	if (msg.empty() || g_brainRun.load())return;
	if (g_brainThread.joinable()) {
		if (g_brainThreadDone.load())g_brainThread.join();
		else {
			SetStatus("Still busy | wait a moment");
			return;
		}
	}
	if (g_coachThread.joinable() && g_coachThreadDone.load())g_coachThread.join();
	BrainAddMsg("user", msg);
	std::string oModel2;
	{ std::lock_guard<std::mutex> lk2(g_cfgStrMtx);oModel2 = g_ollamaModel; }
	if (g_useOllama && OllamaPing() && !OllamaHasModel(oModel2)) {
		SetStatus("Model " + oModel2 + " missing | it will download itself");
	}
	g_brainCancel.store(false);
	g_brainThreadDone.store(false);
	g_brainRun.store(true);
	g_brainThread = std::thread(BrainWorker);
}
static void BrainStop() {
	g_brainCancel.store(true);
	g_brainRun.store(false);
	for (int i = 0;i < 1000 && !g_brainThreadDone.load();++i)Sleep(5);
	if (g_brainThread.joinable() && g_brainThreadDone.load())g_brainThread.join();
	g_brainThinking.store(false);
	SetStatus("Stopped");
}
static void RadarPublishFrame(CompositingPipeline& comp) {
	if (!comp.GetScene())return;
	static double s_lastPub = 0.0;
	double nowT = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
	if (nowT - s_lastPub < 0.25)return;
	s_lastPub = nowT;
	if (!comp.DownscaleFrame(g_dev, g_ctx, comp.GetScene(), kRadarDsW, kRadarDsH, &g_radarSlot))return;
	if (!g_radarStaging || g_radarStagingW != kRadarDsW || g_radarStagingH != kRadarDsH) {
		g_radarStaging.Reset();
		D3D11_TEXTURE2D_DESC st{};
		st.Width = kRadarDsW;st.Height = kRadarDsH;st.MipLevels = 1;st.ArraySize = 1;
		st.Format = DXGI_FORMAT_B8G8R8A8_UNORM;st.SampleDesc.Count = 1;
		st.Usage = D3D11_USAGE_STAGING;st.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		if (FAILED(g_dev->CreateTexture2D(&st, nullptr, &g_radarStaging)))return;
		g_radarStagingW = kRadarDsW;g_radarStagingH = kRadarDsH;
	}
	g_ctx->CopyResource(g_radarStaging.Get(), g_radarSlot.tex.Get());
	D3D11_MAPPED_SUBRESOURCE mp{};
	if (SUCCEEDED(g_ctx->Map(g_radarStaging.Get(), 0, D3D11_MAP_READ, 0, &mp)) && mp.pData) {
		int fw = kRadarDsW, fh = kRadarDsH;
		std::vector<uint8_t> bytes((size_t)fw * fh * 4);
		for (int y = 0;y < fh;++y)
			memcpy(&bytes[(size_t)y * fw * 4], (const uint8_t*)mp.pData + (size_t)y * mp.RowPitch, (size_t)fw * 4);
		g_ctx->Unmap(g_radarStaging.Get(), 0);
		{
			std::lock_guard<std::mutex> lk(g_radarFrameMtx);
			g_radarFrameBytes = std::move(bytes);
			g_radarFrameW = fw;g_radarFrameH = fh;
		}
		g_radarFrameVer.store(g_radarFrameVer.load() + 1);
	}
	else {
		g_ctx->Unmap(g_radarStaging.Get(), 0);
	}
}
static void ObjDbgInit() {
	if (!g_objDbgDir.empty())return;
	wchar_t buf[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, buf, MAX_PATH);
	std::wstring exe(buf);
	size_t s = exe.find_last_of(L"\\/");
	g_objDbgDir = (s != std::wstring::npos ? exe.substr(0, s + 1) : L"") + L"models\\";
	std::error_code ec;
	std::filesystem::create_directories(g_objDbgDir, ec);
	std::string logN;
	{
		logN = WtoA((g_objDbgDir + L"obj_gen.log").c_str());
	}
	std::ofstream f(logN);
	if (f)f << "--- session ---\n";
}
static void ObjDbgLog(const std::string& line) {
	ObjDbgInit();
	std::string logN;
	{
		logN = WtoA((g_objDbgDir + L"obj_gen.log").c_str());
	}
	std::ofstream f(logN, std::ios::app);
	if (f)f << line << "\n";
}
static void SaveObjDebug(const std::vector<uint8_t>& bgra, int w, int h, int attempt) {
	ObjDbgInit();
	if (bgra.empty() || w <= 0 || h <= 0)return;
	std::wstring path = g_objDbgDir + L"obj_debug_" + std::to_wstring(attempt) + L".bmp";
	std::string pathN;
	{
		pathN = WtoA(path.c_str());
	}
	std::ofstream f(pathN, std::ios::binary);
	if (!f)return;
	uint32_t rowSize = ((uint32_t)w * 3 + 3) & ~3u;
	uint32_t dataSize = rowSize * (uint32_t)h;
	uint8_t hdr[54] = {};
	hdr[0] = 'B';hdr[1] = 'M';
	uint32_t fileSize = 54 + dataSize;
	memcpy(hdr + 2, &fileSize, 4);
	hdr[10] = 54;
	hdr[14] = 40;
	memcpy(hdr + 18, &w, 4);memcpy(hdr + 22, &h, 4);
	hdr[26] = 1;hdr[28] = 24;
	memcpy(hdr + 34, &dataSize, 4);
	f.write((const char*)hdr, 54);
	std::vector<uint8_t> row(rowSize, 0);
	for (int y = h - 1;y >= 0;--y) {
		for (int x = 0;x < w;++x) {
			const uint8_t* p = &bgra[((size_t)y * w + x) * 4];
			row[(size_t)x * 3 + 0] = p[2];
			row[(size_t)x * 3 + 1] = p[1];
			row[(size_t)x * 3 + 2] = p[0];
		}
		f.write((const char*)row.data(), (std::streamsize)rowSize);
	}
	std::string narrow;
	g_objDbgPath = WtoA(path.c_str());
}
static void ObjMainLoopTick() {
	if (!g_objReady.load())return;
	if (g_objDirty.load()) {
		std::lock_guard<std::mutex> lk(g_objMtx);
		if (g_objSrv) { g_objSrv->Release();g_objSrv = nullptr; }
		if (!g_objPatch.empty() && g_objPW > 0 && g_objPH > 0 && g_dev) {
			std::vector<uint8_t> pm(g_objPatch.size());
			for (size_t i = 0;i < g_objPatch.size();i += 4) {
				float a = g_objPatch[i + 3] / 255.0f;
				pm[i + 0] = (uint8_t)(g_objPatch[i + 0] * a + 0.5f);
				pm[i + 1] = (uint8_t)(g_objPatch[i + 1] * a + 0.5f);
				pm[i + 2] = (uint8_t)(g_objPatch[i + 2] * a + 0.5f);
				pm[i + 3] = g_objPatch[i + 3];
			}
			D3D11_TEXTURE2D_DESC td{};
			td.Width = (UINT)g_objPW;td.Height = (UINT)g_objPH;td.MipLevels = 1;td.ArraySize = 1;
			td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DEFAULT;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			D3D11_SUBRESOURCE_DATA srd{};srd.pSysMem = pm.data();srd.SysMemPitch = g_objPW * 4;
			ID3D11Texture2D* tex = nullptr;
			if (SUCCEEDED(g_dev->CreateTexture2D(&td, &srd, &tex))) {
				g_dev->CreateShaderResourceView(tex, nullptr, &g_objSrv);
				tex->Release();
			}
		}
		g_objDirty.store(false);
	}
}
static void LiveApplyResult(AppConfig& cfg, CompositingPipeline& comp) {
	int ver = g_liveOutVer.load();
	if (ver == 0 || ver == g_liveShownVer.load())return;
	{
		std::lock_guard<std::mutex> lk(g_liveOutMtx);
		if (!g_liveOutReady)return;
		std::vector<uint8_t> frame = g_liveOuts[(g_liveOutSlot + 2) % 3];
		int fw = g_liveOutW, fh = g_liveOutH;
		g_liveOutReady = false;
		if (!frame.empty() && fw > 0 && fh > 0) {
			int sw = (int)g_W, sh = (int)g_H;
			if (sw > 0 && sh > 0 && (fw != sw || fh != sh)) {
				std::vector<uint8_t> resized((size_t)sw * sh * 4);
				for (int y = 0;y < sh;++y) {
					float syf = ((float)y + 0.5f) * (float)fh / (float)sh - 0.5f;
					int sy0 = (int)std::floor(syf);
					float fy = syf - (float)sy0;
					sy0 = std::clamp(sy0, 0, fh - 1);
					int sy1 = std::clamp(sy0 + 1, 0, fh - 1);
					for (int x = 0;x < sw;++x) {
						float sxf = ((float)x + 0.5f) * (float)fw / (float)sw - 0.5f;
						int sx0 = (int)std::floor(sxf);
						float fx = sxf - (float)sx0;
						sx0 = std::clamp(sx0, 0, fw - 1);
						int sx1 = std::clamp(sx0 + 1, 0, fw - 1);
						const uint8_t* p00 = &frame[((size_t)sy0 * fw + sx0) * 4];
						const uint8_t* p01 = &frame[((size_t)sy0 * fw + sx1) * 4];
						const uint8_t* p10 = &frame[((size_t)sy1 * fw + sx0) * 4];
						const uint8_t* p11 = &frame[((size_t)sy1 * fw + sx1) * 4];
						uint8_t* d = &resized[((size_t)y * sw + x) * 4];
						for (int c = 0;c < 4;++c) {
							float top = p00[c] * (1.0f - fx) + p01[c] * fx;
							float bot = p10[c] * (1.0f - fx) + p11[c] * fx;
							d[c] = (uint8_t)(top * (1.0f - fy) + bot * fy + 0.5f);
						}
					}
				}
				frame = std::move(resized);
				fw = sw;fh = sh;
			}
			comp.UpdateStyleTex(g_dev, g_ctx, frame, fw, fh);
		}
	}
	cfg.fx.upscaleSD = true;
	cfg.fx.upscaleStrength = g_liveBlend;
	cfg.fx.splitScreenEnabled = false;
	g_sdApplied = true;
	g_liveShownVer.store(ver);
}
static std::atomic<bool> g_capActive{ true };
static std::atomic<bool> g_capPixelsNeeded{ true };
class ScreenCapture {
public:
	bool Init(ID3D11Device* dev, IDXGIOutput* out) {
		(void)dev;
		std::lock_guard<std::mutex> lk(m_mtx);
		m_dup.Reset();
		m_outDesc = {};
		if (!out)return false;
		out->GetDesc(&m_outDesc);
		ComPtr<IDXGIAdapter> adapter;
		if (FAILED(out->GetParent(IID_PPV_ARGS(&adapter))))return false;
		D3D_FEATURE_LEVEL fl;
		const D3D_FEATURE_LEVEL lvls[] = { D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_0 };
		if (FAILED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, lvls, 2, D3D11_SDK_VERSION, &m_dev, &fl, &m_ctx)))return false;
		ComPtr<IDXGIOutput1> o1;
		if (FAILED(out->QueryInterface(IID_PPV_ARGS(&o1))))return false;
		if (FAILED(o1->DuplicateOutput(m_dev.Get(), &m_dup))) { m_dup.Reset();return false; }
		m_stg.Reset();m_stgW = 0;m_stgH = 0;
		return true;
	}
	void Start() {
		if (m_thread.joinable())return;
		m_run.store(true);
		m_thread = std::thread([this] {Worker();});
	}
	void Stop() {
		m_run.store(false);
		if (m_thread.joinable())m_thread.join();
		std::lock_guard<std::mutex> lk(m_mtx);
		m_dup.Reset();
		m_stg.Reset();
		m_ctx.Reset();
		m_dev.Reset();
	}
	bool TryGetFrame(ID3D11Device* dev, ID3D11DeviceContext* ctx, CompositingPipeline& comp, DepthEngine& depth, const RECT& r) {
		(void)r;
		std::vector<uint8_t> px;
		int w = 0, h = 0;
		{
			std::lock_guard<std::mutex> lk(m_pubMtx);
			if (!m_new)return false;
			px.swap(m_px);
			w = m_pw;h = m_ph;
			m_new = false;
		}
		if (px.empty() || w <= 0 || h <= 0)return false;
		if (!m_tex || m_texW != w || m_texH != h) {
			m_tex.Reset();
			D3D11_TEXTURE2D_DESC td{};
			td.Width = (UINT)w;td.Height = (UINT)h;td.MipLevels = 1;td.ArraySize = 1;
			td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DEFAULT;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			if (FAILED(dev->CreateTexture2D(&td, nullptr, &m_tex)))return false;
			m_texW = w;m_texH = h;
		}
		ctx->UpdateSubresource(m_tex.Get(), 0, nullptr, px.data(), (UINT)w * 4, 0);
		RECT full{ 0,0,w,h };
		comp.UpdateSceneCropped(dev, ctx, m_tex.Get(), full);
		if (comp.GetScene())depth.Submit(dev, ctx, comp.GetScene());
		return true;
	}
	bool Valid()const { std::lock_guard<std::mutex> lk(m_mtx);return m_dup != nullptr; }
private:
	void Worker() {
		while (m_run.load()) {
			if (!g_capActive.load()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
				continue;
			}
			ComPtr<IDXGIOutputDuplication> dup;
			ComPtr<ID3D11DeviceContext> ctx;
			DXGI_OUTPUT_DESC desc{};
			{
				std::lock_guard<std::mutex> lk(m_mtx);
				dup = m_dup;
				ctx = m_ctx;
				desc = m_outDesc;
			}
			if (!dup || !ctx) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			ComPtr<IDXGIResource> res;
			DXGI_OUTDUPL_FRAME_INFO fi{};
			HRESULT hr = dup->AcquireNextFrame(0, &fi, &res);
			if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
				std::this_thread::sleep_for(std::chrono::microseconds(50));
				continue;
			}
			if (hr == DXGI_ERROR_ACCESS_LOST) {
				std::lock_guard<std::mutex> lk(m_mtx);
				m_dup.Reset();
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
				continue;
			}
			if (FAILED(hr)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
			{
				double now = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
				if (m_fpsT0 == 0.0)m_fpsT0 = now;
				m_fpsFrames++;
				double fpsDt = now - m_fpsT0;
				if (fpsDt >= 0.25) {
					g_gameFps.store((double)m_fpsFrames / fpsDt);
					m_fpsFrames = 0;
					m_fpsT0 = now;
				}
			}
			if (!g_capPixelsNeeded.load()) {
				dup->ReleaseFrame();
				std::this_thread::sleep_for(std::chrono::microseconds(100));
				continue;
			}
			ComPtr<ID3D11Texture2D> tex;
			if (SUCCEEDED(res.As(&tex)) && tex) {
				D3D11_TEXTURE2D_DESC sd{};
				tex->GetDesc(&sd);
				if (!m_stg || m_stgW != sd.Width || m_stgH != sd.Height) {
					m_stg.Reset();
					D3D11_TEXTURE2D_DESC st = sd;
					st.Usage = D3D11_USAGE_STAGING;
					st.BindFlags = 0;
					st.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
					st.MiscFlags = 0;
					if (SUCCEEDED(m_dev->CreateTexture2D(&st, nullptr, &m_stg))) { m_stgW = sd.Width;m_stgH = sd.Height; }
				}
				if (m_stg) {
					ctx->CopyResource(m_stg.Get(), tex.Get());
					D3D11_MAPPED_SUBRESOURCE mp{};
					if (SUCCEEDED(ctx->Map(m_stg.Get(), 0, D3D11_MAP_READ, 0, &mp)) && mp.pData) {
						HWND hwnd = g_robloxHwnd;
						RECT wr{};
						if (hwnd && GetWindowRect(hwnd, &wr)) {
							RECT desk = desc.DesktopCoordinates;
							LONG l = std::max(wr.left, desk.left), t = std::max(wr.top, desk.top);
							LONG rr = std::min(wr.right, desk.right), b = std::min(wr.bottom, desk.bottom);
							int w = (int)(rr - l), h = (int)(b - t);
							if (w >= 64 && h >= 64) {
								int ox = (int)(l - desk.left), oy = (int)(t - desk.top);
								float scale = std::min(1.0f, std::min(1920.0f / w, 1080.0f / h));
								int sw = std::max(64, (int)(w * scale));
								int sh = std::max(64, (int)(h * scale));
								std::vector<uint8_t> px((size_t)sw * sh * 4);
								const uint8_t* srcBase = (const uint8_t*)mp.pData + (size_t)oy * mp.RowPitch + (size_t)ox * 4;
								for (int y = 0;y < sh;++y) {
									int sy = std::min(h - 1, (int)(y / scale));
									memcpy(&px[(size_t)y * sw * 4], srcBase + (size_t)sy * mp.RowPitch, (size_t)sw * 4);
								}
								if (FrameChanged(px, sw, sh)) {
									std::lock_guard<std::mutex> lk(m_pubMtx);
									m_px.swap(px);
									m_pw = sw;m_ph = sh;
									m_new = true;
								}
							}
						}
						ctx->Unmap(m_stg.Get(), 0);
					}
				}
			}
			dup->ReleaseFrame();
		}
	}
	bool FrameChanged(const std::vector<uint8_t>& px, int w, int h) {
		if (m_prev.size() != px.size()) {
			m_prev = px;
			return true;
		}
		size_t row = (size_t)w * 4;
		const uint8_t* c = px.data();
		uint8_t* p = m_prev.data();
		bool changed = false;
		for (int y = 0;y < h && !changed;y += 4) {
			const uint32_t* c32 = reinterpret_cast<const uint32_t*>(c + (size_t)y * row);
			uint32_t* p32 = reinterpret_cast<uint32_t*>(p + (size_t)y * row);
			for (int x = 0;x < w;x += 8) {
				if (c32[x] != p32[x]) { changed = true;break; }
				p32[x] = c32[x];
			}
		}
		if (changed) {
			for (int y = 0;y < h;y += 2)
				memcpy(p + (size_t)y * row, c + (size_t)y * row, row);
		}
		return changed;
	}
	std::atomic<bool> m_run{ false };
	double m_fpsT0 = 0.0;
	int m_fpsFrames = 0;
	std::thread m_thread;
	mutable std::mutex m_mtx;
	std::mutex m_pubMtx;
	ComPtr<ID3D11Device> m_dev;
	ComPtr<ID3D11DeviceContext> m_ctx;
	ComPtr<IDXGIOutputDuplication> m_dup;
	DXGI_OUTPUT_DESC m_outDesc{};
	ComPtr<ID3D11Texture2D> m_stg;
	UINT m_stgW = 0, m_stgH = 0;
	std::vector<uint8_t> m_px;
	std::vector<uint8_t> m_prev;
	int m_pw = 0, m_ph = 0;
	bool m_new = false;
	int m_texW = 0, m_texH = 0;
	ComPtr<ID3D11Texture2D> m_tex;
};
static bool InitCaptureForRobloxWindow(ScreenCapture& cap, IDXGIAdapter1* adapter, ID3D11Device* dev, HWND robloxHwnd) {
	cap.Stop();
	g_captureMonitor = nullptr;
	if (!adapter || !dev || !robloxHwnd)return false;
	HMONITOR targetMon = MonitorFromWindow(robloxHwnd, MONITOR_DEFAULTTONEAREST);
	if (!targetMon)return false;
	for (UINT i = 0;;++i) {
		ComPtr<IDXGIOutput> out;
		if (adapter->EnumOutputs(i, &out) == DXGI_ERROR_NOT_FOUND)break;
		DXGI_OUTPUT_DESC od{};
		if (FAILED(out->GetDesc(&od)))continue;
		if (od.Monitor == targetMon) {
			bool ok = cap.Init(dev, out.Get());
			if (ok) { g_captureMonitor = targetMon;cap.Start();return true; }
		}
	}
	ComPtr<IDXGIOutput> out0;
	if (SUCCEEDED(adapter->EnumOutputs(0, &out0))) {
		bool ok = cap.Init(dev, out0.Get());
		if (ok) { g_captureMonitor = targetMon;cap.Start();return true; }
	}
	return false;
}
static void CreateRTV() { if (!g_sc || !g_dev)return;ID3D11Texture2D* bb = nullptr;if (SUCCEEDED(g_sc->GetBuffer(0, IID_PPV_ARGS(&bb))) && bb) { g_dev->CreateRenderTargetView(bb, nullptr, &g_rtv);bb->Release(); } }
static void DropRTV() { if (g_rtv) { g_rtv->Release();g_rtv = nullptr; } }
static HICON g_appIcon = nullptr;
static HICON g_appIconSm = nullptr;
static HICON TryLoadIconFile(const std::wstring& p, int px) {
	if (p.empty() || !std::filesystem::exists(p))return nullptr;
	return (HICON)LoadImageW(nullptr, p.c_str(), IMAGE_ICON, px, px, LR_LOADFROMFILE);
}
static void LoadAppIcons() {
	wchar_t exePath[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	std::wstring exeDir(exePath);
	size_t slash = exeDir.find_last_of(L"\\/");
	exeDir = (slash != std::wstring::npos) ? exeDir.substr(0, slash + 1) : L"";
	std::wstring cwd = std::filesystem::current_path().wstring() + L"\\";
	const wchar_t* names[] = {
	L"Resource Files\\studworkslogo.ico",
	L"Resource Files\\studworkslog.ico",
	L"studworkslogo.ico",
	L"studworkslog.ico",
	L"studreshader.ico",
	L"app.ico"
	};
	for (int i = 0;i < 6;++i) {
		HICON bigIcon = TryLoadIconFile(cwd + names[i], 32);
		if (!bigIcon)bigIcon = TryLoadIconFile(exeDir + names[i], 32);
		if (bigIcon) {
			g_appIcon = bigIcon;
			HICON smallIcon = TryLoadIconFile(cwd + names[i], 16);
			if (!smallIcon)smallIcon = TryLoadIconFile(exeDir + names[i], 16);
			g_appIconSm = smallIcon ? smallIcon : CopyIcon(bigIcon);
			return;
		}
	}
	HICON largeIcon = nullptr;
	HICON smallIcon = nullptr;
	if (ExtractIconExW(exePath, 0, &largeIcon, &smallIcon, 1) > 0) {
		g_appIcon = largeIcon ? largeIcon : smallIcon;
		g_appIconSm = smallIcon ? smallIcon : (g_appIcon ? CopyIcon(g_appIcon) : nullptr);
		return;
	}
	SHFILEINFOW sfi{};
	if (SHGetFileInfoW(exePath, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON) && sfi.hIcon) {
		g_appIcon = sfi.hIcon;
		g_appIconSm = sfi.hIcon;
		return;
	}
	g_appIcon = LoadIconW(nullptr, IDI_APPLICATION);
	g_appIconSm = LoadIconW(nullptr, IDI_APPLICATION);
}
static bool CreateTextureFromHiconTo(ID3D11Device* dev, HICON hIcon, ID3D11ShaderResourceView** outSrv, int* outW, int* outH) {
	if (!dev || !hIcon || !outSrv)return false;const int target = 256;BITMAPV5HEADER bi{};bi.bV5Size = sizeof(bi);bi.bV5Width = target;bi.bV5Height = -target;bi.bV5Planes = 1;bi.bV5BitCount = 32;bi.bV5Compression = BI_BITFIELDS;bi.bV5RedMask = 0x00FF0000;bi.bV5GreenMask = 0x0000FF00;bi.bV5BlueMask = 0x000000FF;bi.bV5AlphaMask = 0xFF000000;void* bits = nullptr;HDC hdc = GetDC(nullptr);if (!hdc)return false;
	HBITMAP dib = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &bits, nullptr, 0);if (!dib || !bits) { if (dib)DeleteObject(dib);ReleaseDC(nullptr, hdc);return false; }
	HDC mem = CreateCompatibleDC(hdc);if (!mem) { DeleteObject(dib);ReleaseDC(nullptr, hdc);return false; }HGDIOBJ old = SelectObject(mem, dib);if (!old || old == HGDI_ERROR) { DeleteDC(mem);DeleteObject(dib);ReleaseDC(nullptr, hdc);return false; }
	RECT rc{ 0,0,target,target };HBRUSH clear = CreateSolidBrush(RGB(0, 0, 0));if (clear) { FillRect(mem, &rc, clear);DeleteObject(clear); }DrawIconEx(mem, 0, 0, hIcon, target, target, 0, nullptr, DI_NORMAL);
	std::vector<uint8_t> pix(target * target * 4);memcpy(pix.data(), bits, pix.size());SelectObject(mem, old);DeleteDC(mem);DeleteObject(dib);ReleaseDC(nullptr, hdc);
	D3D11_TEXTURE2D_DESC td{};td.Width = target;td.Height = target;td.MipLevels = 1;td.ArraySize = 1;td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count = 1;td.Usage = D3D11_USAGE_DEFAULT;td.BindFlags = D3D11_BIND_SHADER_RESOURCE;td.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA srd{};srd.pSysMem = pix.data();srd.SysMemPitch = target * 4;ID3D11Texture2D* tex = nullptr;if (FAILED(dev->CreateTexture2D(&td, &srd, &tex)) || !tex)return false;HRESULT hr = dev->CreateShaderResourceView(tex, nullptr, outSrv);tex->Release();if (FAILED(hr))return false;if (outW)*outW = target;if (outH)*outH = target;return true;
}
static bool CreateIconTextureFromHicon(ID3D11Device* dev, HICON hIcon) { if (g_logoSrv) { g_logoSrv->Release();g_logoSrv = nullptr; }g_logoW = 0;g_logoH = 0;return CreateTextureFromHiconTo(dev, hIcon, &g_logoSrv, &g_logoW, &g_logoH); }
static bool LoadLogoTexture(ID3D11Device* dev) {
	wchar_t exePath[MAX_PATH] = {};GetModuleFileNameW(nullptr, exePath, MAX_PATH);std::wstring exeDir(exePath);size_t slash = exeDir.find_last_of(L"\\/");exeDir = (slash != std::wstring::npos) ? exeDir.substr(0, slash + 1) : L".\\";
	std::wstring cwd = std::filesystem::current_path().wstring() + L"\\";std::vector<std::wstring> paths = { cwd + L"Resource Files\\studworkslogo.ico",cwd + L"Resource Files\\studworkslog.ico",cwd + L"studworkslogo.ico",cwd + L"studworkslog.ico",exeDir + L"Resource Files\\studworkslogo.ico",exeDir + L"Resource Files\\studworkslog.ico",exeDir + L"studworkslogo.ico",exeDir + L"studworkslog.ico" };
	HICON hIcon = nullptr;for (const auto& p : paths) { if (!std::filesystem::exists(p))continue;hIcon = reinterpret_cast<HICON>(LoadImageW(nullptr, p.c_str(), IMAGE_ICON, 256, 256, LR_LOADFROMFILE));if (hIcon)break; }
	if (!hIcon) { HICON large = nullptr;ExtractIconExW(exePath, 0, &large, nullptr, 1);hIcon = large; }
	if (!hIcon) { SHFILEINFOW sfi{};if (SHGetFileInfoW(exePath, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON))hIcon = sfi.hIcon; }
	if (!hIcon)hIcon = g_appIcon ? CopyIcon(g_appIcon) : LoadIconW(nullptr, IDI_APPLICATION);if (!hIcon)return false;
	bool ok = CreateIconTextureFromHicon(dev, hIcon);DestroyIcon(hIcon);return ok;
}
static bool LoadDiscordTexture(ID3D11Device* dev) { if (!dev)return false;if (g_discordSrv)return true;std::wstring cachePath = std::filesystem::current_path().wstring() + L"\\discord_public_icon.ico";if (!std::filesystem::exists(cachePath))return false;HICON hIcon = reinterpret_cast<HICON>(LoadImageW(nullptr, cachePath.c_str(), IMAGE_ICON, 256, 256, LR_LOADFROMFILE));if (!hIcon)return false;if (g_discordSrv) { g_discordSrv->Release();g_discordSrv = nullptr; }bool ok = CreateTextureFromHiconTo(dev, hIcon, &g_discordSrv, &g_discordW, &g_discordH);DestroyIcon(hIcon);return ok; }
static bool CreateDeviceD3D(HWND hwnd, int w, int h) {
	g_W = w > 0 ? static_cast<UINT>(w) : 1920;g_H = h > 0 ? static_cast<UINT>(h) : 1080;ComPtr<IDXGIFactory2> fac2;if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&fac2))))return false;ComPtr<IDXGIFactory6> fac6;fac2.As(&fac6);
	if (g_targetMonitor) { for (UINT ai = 0;;++ai) { ComPtr<IDXGIAdapter1> a;HRESULT ahr = fac6 ? fac6->EnumAdapterByGpuPreference(ai, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&a)) : fac2->EnumAdapters1(ai, &a);if (ahr == DXGI_ERROR_NOT_FOUND)break;if (!a)continue;DXGI_ADAPTER_DESC1 ad{};a->GetDesc1(&ad);if (ad.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)continue;for (UINT oi = 0;;++oi) { ComPtr<IDXGIOutput> out;if (a->EnumOutputs(oi, &out) == DXGI_ERROR_NOT_FOUND)break;DXGI_OUTPUT_DESC od{};if (SUCCEEDED(out->GetDesc(&od)) && od.Monitor == g_targetMonitor) { g_dxgiAdapter = a;break; } }if (g_dxgiAdapter)break; } }
	if (!g_dxgiAdapter) { if (fac6)fac6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&g_dxgiAdapter)); }
	if (!g_dxgiAdapter) { ComPtr<IDXGIAdapter> a0;if (SUCCEEDED(fac2->EnumAdapters(0, &a0)))a0.As(&g_dxgiAdapter); }
	D3D_FEATURE_LEVEL fl;const D3D_FEATURE_LEVEL lvls[] = { D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_0 };
	HRESULT hr = g_dxgiAdapter ? D3D11CreateDevice(g_dxgiAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, lvls, 2, D3D11_SDK_VERSION, &g_dev, &fl, &g_ctx) : D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, lvls, 2, D3D11_SDK_VERSION, &g_dev, &fl, &g_ctx);
	if (FAILED(hr) || !g_dev)return false;ComPtr<IDXGIDevice> dxgiDev;if (FAILED(g_dev->QueryInterface(IID_PPV_ARGS(&dxgiDev))))return false;
	DXGI_SWAP_CHAIN_DESC1 scd{};scd.Width = g_W;scd.Height = g_H;scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;scd.SampleDesc.Count = 1;scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;scd.BufferCount = 3;scd.Scaling = DXGI_SCALING_STRETCH;scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;scd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
	hr = fac2->CreateSwapChainForComposition(g_dev, &scd, nullptr, &g_sc);if (FAILED(hr) || !g_sc)return false;
	hr = DCompositionCreateDevice(dxgiDev.Get(), IID_PPV_ARGS(&g_dcd));if (FAILED(hr) || !g_dcd)return false;
	if (FAILED(g_dcd->CreateTargetForHwnd(hwnd, TRUE, &g_dct)) || !g_dct)return false;if (FAILED(g_dcd->CreateVisual(&g_dcv)) || !g_dcv)return false;
	g_dcv->SetContent(g_sc);g_dct->SetRoot(g_dcv);g_dcd->Commit();CreateRTV();return g_rtv != nullptr;
}
static void CleanupDeviceD3D() { DropRTV();if (g_logoSrv) { g_logoSrv->Release();g_logoSrv = nullptr; }if (g_discordSrv) { g_discordSrv->Release();g_discordSrv = nullptr; }if (g_generatedSrv) { g_generatedSrv->Release();g_generatedSrv = nullptr; }if (g_dcv) { g_dcv->Release();g_dcv = nullptr; }if (g_dct) { g_dct->Release();g_dct = nullptr; }if (g_dcd) { g_dcd->Release();g_dcd = nullptr; }if (g_sc) { g_sc->Release();g_sc = nullptr; }if (g_ctx) { g_ctx->Release();g_ctx = nullptr; }if (g_dev) { g_dev->Release();g_dev = nullptr; } }
static void ApplyGuiTheme(int theme) {
	ImGuiStyle& s = ImGui::GetStyle();ImVec4* c = s.Colors;
	s.AntiAliasedLines = true;s.AntiAliasedFill = true;s.WindowRounding = 14.0f;s.ChildRounding = 10.0f;s.FrameRounding = 8.0f;s.PopupRounding = 8.0f;s.ScrollbarRounding = 10.0f;s.GrabRounding = 6.0f;s.TabRounding = 6.0f;
	s.WindowPadding = { 14,12 };s.FramePadding = { 9,7 };s.ItemSpacing = { 8,7 };s.ItemInnerSpacing = { 5,4 };s.IndentSpacing = 14;s.ScrollbarSize = 10;s.GrabMinSize = 10;s.WindowBorderSize = 1;s.FrameBorderSize = 0;s.ChildBorderSize = 0;
	c[ImGuiCol_WindowBg] = { 0.015f,0.015f,0.020f,1.0f };c[ImGuiCol_ChildBg] = { 0.040f,0.040f,0.050f,1.0f };c[ImGuiCol_PopupBg] = { 0.025f,0.025f,0.030f,1.0f };c[ImGuiCol_Border] = { 0.12f,0.12f,0.14f,1.0f };c[ImGuiCol_BorderShadow] = { 0,0,0,0 };
	c[ImGuiCol_FrameBg] = { 0.070f,0.070f,0.080f,1.0f };c[ImGuiCol_FrameBgHovered] = { 0.11f,0.11f,0.13f,1.0f };c[ImGuiCol_FrameBgActive] = { 0.15f,0.15f,0.18f,1.0f };
	c[ImGuiCol_TitleBg] = c[ImGuiCol_TitleBgActive] = c[ImGuiCol_TitleBgCollapsed] = { 0.01f,0.01f,0.02f,1 };c[ImGuiCol_Button] = { 0.080f,0.080f,0.10f,1.0f };c[ImGuiCol_ButtonHovered] = { 0.14f,0.14f,0.17f,1.0f };c[ImGuiCol_ButtonActive] = { 0.20f,0.20f,0.23f,1.0f };
	c[ImGuiCol_Header] = { 0.10f,0.10f,0.12f,1 };c[ImGuiCol_HeaderHovered] = { 0.14f,0.14f,0.17f,1 };c[ImGuiCol_HeaderActive] = { 0.20f,0.20f,0.24f,1 };c[ImGuiCol_Separator] = { 0.10f,0.10f,0.12f,1 };c[ImGuiCol_Tab] = { 0.05f,0.05f,0.06f,1 };c[ImGuiCol_TabHovered] = { 0.13f,0.13f,0.16f,1 };c[ImGuiCol_TabActive] = { 0.16f,0.16f,0.19f,1 };c[ImGuiCol_Text] = { 0.85f,0.85f,0.90f,1 };c[ImGuiCol_TextDisabled] = { 0.40f,0.40f,0.46f,1 };
	c[ImGuiCol_ScrollbarBg] = { 0,0,0,0 };c[ImGuiCol_ScrollbarGrab] = { 0.18f,0.18f,0.21f,1 };c[ImGuiCol_ScrollbarGrabHovered] = { 0.24f,0.24f,0.28f,1 };c[ImGuiCol_ScrollbarGrabActive] = { 0.32f,0.32f,0.36f,1 };c[ImGuiCol_ModalWindowDimBg] = { 0,0,0,0.55f };
	if (theme == 0) { g_accentCol = IM_COL32(0, 200, 255, 255);g_accentDimCol = IM_COL32(0, 120, 185, 255);c[ImGuiCol_CheckMark] = { 0.00f,0.80f,1.00f,1 };c[ImGuiCol_SliderGrab] = { 0.00f,0.65f,0.95f,1 };c[ImGuiCol_SliderGrabActive] = { 0.00f,0.85f,1.00f,1 }; }
	else if (theme == 1) { g_accentCol = IM_COL32(255, 0, 180, 255);g_accentDimCol = IM_COL32(160, 0, 100, 255);c[ImGuiCol_CheckMark] = { 1.0f,0.2f,0.8f,1 };c[ImGuiCol_SliderGrab] = { 0.92f,0.12f,0.72f,1 };c[ImGuiCol_SliderGrabActive] = { 1.0f,0.25f,0.85f,1 }; }
	else if (theme == 2) { g_accentCol = IM_COL32(0, 255, 100, 255);g_accentDimCol = IM_COL32(0, 155, 60, 255);c[ImGuiCol_CheckMark] = { 0.0f,1.0f,0.4f,1 };c[ImGuiCol_SliderGrab] = { 0.0f,0.82f,0.35f,1 };c[ImGuiCol_SliderGrabActive] = { 0.0f,1.0f,0.45f,1 }; }
	else { g_accentCol = IM_COL32(255, 140, 0, 255);g_accentDimCol = IM_COL32(180, 70, 0, 255);c[ImGuiCol_CheckMark] = { 1.0f,0.55f,0.0f,1 };c[ImGuiCol_SliderGrab] = { 0.90f,0.45f,0.0f,1 };c[ImGuiCol_SliderGrabActive] = { 1.0f,0.55f,0.0f,1 }; }
}
static float AnimF(ImGuiID id, float target, float speed) { ImGuiStorage* st = ImGui::GetStateStorage();float v = st->GetFloat(id, target);v += (target - v) * std::min(1.0f, ImGui::GetIO().DeltaTime * speed);st->SetFloat(id, v);return v; }
static float EaseOutCubic(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
static void SectionHeader(const char* title, const char* sub) {
	ImDrawList* dl = ImGui::GetWindowDrawList();ImVec2 p = ImGui::GetCursorScreenPos();float w = ImGui::GetContentRegionAvail().x;
	dl->AddRectFilledMultiColor(p, { p.x + w,p.y + 34 }, IM_COL32(16, 16, 20, 255), IM_COL32(10, 10, 14, 255), IM_COL32(8, 8, 12, 255), IM_COL32(14, 14, 18, 255));
	dl->AddRectFilled(p, { p.x + 3,p.y + 34 }, g_accentCol);dl->AddText({ p.x + 12,p.y + 8 }, IM_COL32(230, 230, 235, 255), title);dl->AddRectFilled({ p.x,p.y + 33 }, { p.x + w,p.y + 34 }, IM_COL32(35, 35, 40, 255));
	float sh = fmodf(static_cast<float>(ImGui::GetTime()) * 0.8f, 1.0f);float seg = w * 0.35f;float cx = p.x + sh * (w + seg) - seg;dl->AddRectFilledMultiColor({ cx,p.y + 33 }, { cx + seg,p.y + 34 }, g_accentCol & 0x00FFFFFF, g_accentCol, g_accentCol, g_accentCol & 0x00FFFFFF);
	ImGui::Dummy({ w,34 });ImGui::Spacing();if (sub && sub[0]) { ImGui::PushStyleColor(ImGuiCol_Text, { 0.50f,0.50f,0.56f,1 });ImGui::TextWrapped("%s", sub);ImGui::PopStyleColor();ImGui::Spacing(); }
}
static void GlowSep() { ImDrawList* dl = ImGui::GetWindowDrawList();ImVec2 p = ImGui::GetCursorScreenPos();float w = ImGui::GetContentRegionAvail().x;dl->AddRectFilledMultiColor(p, { p.x + w,p.y + 1 }, IM_COL32(0, 0, 0, 0), IM_COL32(45, 45, 55, 255), IM_COL32(45, 45, 55, 255), IM_COL32(0, 0, 0, 0));ImGui::Dummy({ w,1 });ImGui::Spacing(); }
static bool PremiumSlider(const char* id, const char* label, float* val, float mn, float mx, const char* fmt = "%.2f", const char* desc = nullptr) {
	ImGui::PushID(id);ImDrawList* dl = ImGui::GetWindowDrawList();float w = ImGui::GetContentRegionAvail().x;
	const float LABEL_H = 16.0f, ZONE = 24.0f, TRACK_H = 5.0f, THUMB = 8.0f, PAD = THUMB + 6.0f;const float TOTAL_H = LABEL_H + ZONE + (desc ? 13.0f : 4.0f);
	ImVec2 pos = ImGui::GetCursorScreenPos();dl->AddText({ pos.x,pos.y + 1 }, IM_COL32(185, 185, 195, 255), label);char valStr[64];sprintf_s(valStr, fmt, *val);ImVec2 vsz = ImGui::CalcTextSize(valStr);dl->AddText({ pos.x + w - vsz.x,pos.y + 1 }, g_accentCol, valStr);
	ImGui::SetCursorScreenPos(pos);ImGui::InvisibleButton("##sl", { w,LABEL_H + ZONE });bool active = ImGui::IsItemActive();bool hov = ImGui::IsItemHovered();
	float trkX0 = pos.x + PAD, trkX1 = pos.x + w - PAD, trkW = trkX1 - trkX0;float trkY = pos.y + LABEL_H + (ZONE - TRACK_H) * 0.5f;
	if (active) { float t = std::clamp((ImGui::GetIO().MousePos.x - trkX0) / trkW, 0.0f, 1.0f);*val = mn + (mx - mn) * t; }
	float t = std::clamp((*val - mn) / (mx - mn), 0.0f, 1.0f);float hovA = AnimF(ImGui::GetID("##h"), (hov || active) ? 1.0f : 0.0f, 14.0f);float thumbR = THUMB * (1.0f + 0.18f * hovA);float thumbX = trkX0 + trkW * t, thumbCY = trkY + TRACK_H * 0.5f;
	dl->AddRectFilled({ trkX0,trkY }, { trkX1,trkY + TRACK_H }, IM_COL32(12, 12, 15, 255), 3);if (t > 0.0005f)dl->AddRectFilledMultiColor({ trkX0,trkY }, { thumbX,trkY + TRACK_H }, g_accentDimCol, g_accentCol, g_accentCol, g_accentDimCol);
	if (hovA > 0.01f || active)dl->AddCircleFilled({ thumbX,thumbCY }, thumbR + 7.0f * hovA, (g_accentCol & 0x00FFFFFF) | (static_cast<ImU32>(hovA * 44) << 24));
	dl->AddCircleFilled({ thumbX,thumbCY }, thumbR + 1.5f, IM_COL32(15, 15, 18, 255));dl->AddCircleFilled({ thumbX,thumbCY }, thumbR, IM_COL32(240, 240, 245, 255));
	if (desc)dl->AddText({ pos.x,pos.y + LABEL_H + ZONE + 1 }, IM_COL32(90, 90, 96, 255), desc);ImGui::Dummy({ w,TOTAL_H });ImGui::PopID();return active;
}
static bool PremiumToggle(const char* label, bool* v, const char* desc = nullptr) {
	ImGui::PushID(label);const float H = 20.0f, W = 36.0f, R = H * 0.5f;ImVec2 p = ImGui::GetCursorScreenPos();float totalW = ImGui::GetContentRegionAvail().x;ImVec2 labelSz = ImGui::CalcTextSize(label);float tX = std::min(p.x + labelSz.x + 14.0f, p.x + totalW - W);
	ImGui::InvisibleButton("##tog", { totalW,H });bool clicked = ImGui::IsItemClicked();if (clicked)*v = !*v;bool hov = ImGui::IsItemHovered();ImDrawList* dl = ImGui::GetWindowDrawList();
	float anim = AnimF(ImGui::GetID("##a"), *v ? 1.0f : 0.0f, 16.0f);float hovA = AnimF(ImGui::GetID("##h"), hov ? 1.0f : 0.0f, 12.0f);
	ImVec4 trackOff = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);ImVec4 trackOn = ImGui::ColorConvertU32ToFloat4(g_accentCol);ImVec4 trackCur = ImVec4(trackOff.x + (trackOn.x - trackOff.x) * anim, trackOff.y + (trackOn.y - trackOff.y) * anim, trackOff.z + (trackOn.z - trackOff.z) * anim, 1.0f);
	ImU32 trackCol = ImGui::ColorConvertFloat4ToU32(trackCur);dl->AddRectFilled({ tX,p.y }, { tX + W,p.y + H }, trackCol, R);dl->AddRect({ tX,p.y }, { tX + W,p.y + H }, IM_COL32(0, 0, 0, static_cast<ImU32>(50 + hovA * 50)), R, 0, 1.0f);
	float kX = tX + R + (W - R * 2) * anim;float kR = R - 3.0f + hovA * 0.5f;dl->AddCircleFilled({ kX,p.y + R + 1.0f }, kR + 1.5f, IM_COL32(0, 0, 0, 40));dl->AddCircleFilled({ kX,p.y + R }, kR, anim > 0.5f ? IM_COL32(255, 255, 255, 255) : IM_COL32(145, 145, 155, 255));
	dl->AddText({ p.x,p.y + (H - ImGui::GetTextLineHeight()) * 0.5f }, IM_COL32(205, 205, 212, 255), label);
	if (desc) { dl->AddText({ p.x,p.y + H + 1 }, IM_COL32(88, 88, 94, 255), desc);ImGui::Dummy({ totalW,H + 13.0f }); }
	else ImGui::Dummy({ totalW,H });ImGui::PopID();return clicked;
}
static void DrawMagnifier(ImDrawList* dl, ImVec2 c, float r, ImU32 col) { dl->AddCircle(c, r, col, 20, 1.8f);dl->AddLine({ c.x + r * 0.65f,c.y + r * 0.65f }, { c.x + r * 1.55f,c.y + r * 1.55f }, col, 1.8f); }
static bool SearchBox(const char* id, char* buf, size_t bufSize, float width) {
	ImVec2 pos = ImGui::GetCursorScreenPos();ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 28,8 });ImGui::SetNextItemWidth(width);bool changed = ImGui::InputTextWithHint(id, "Search tools...", buf, bufSize);ImGui::PopStyleVar();
	if (ImGui::IsItemActivated())SetForegroundWindow(g_overlayHwnd);ImDrawList* dl = ImGui::GetWindowDrawList();ImVec2 min = ImGui::GetItemRectMin();ImVec2 max = ImGui::GetItemRectMax();DrawMagnifier(dl, { min.x + 11.0f,(min.y + max.y) * 0.5f - 1.0f }, 4.4f, IM_COL32(120, 120, 128, 255));return changed;
}
enum class TitleBtnKind { Menu, Theme, Minimize, Close };
static void DrawTitleButtonGlyph(ImDrawList* dl, ImVec2 p, float sz, TitleBtnKind kind, ImU32 col) {
	ImVec2 c{ p.x + sz * 0.5f,p.y + sz * 0.5f };switch (kind) {
	case TitleBtnKind::Menu: dl->AddLine({ c.x - 6,c.y - 4 }, { c.x + 6,c.y - 4 }, col, 1.8f);dl->AddLine({ c.x - 6,c.y }, { c.x + 6,c.y }, col, 1.8f);dl->AddLine({ c.x - 6,c.y + 4 }, { c.x + 6,c.y + 4 }, col, 1.8f);break;
	case TitleBtnKind::Theme: dl->AddCircle(c, 6.0f, col, 18, 1.6f);dl->AddLine({ c.x,c.y - 8 }, { c.x,c.y - 11 }, col, 1.6f);dl->AddLine({ c.x,c.y + 8 }, { c.x,c.y + 11 }, col, 1.6f);dl->AddLine({ c.x - 8,c.y }, { c.x - 11,c.y }, col, 1.6f);dl->AddLine({ c.x + 8,c.y }, { c.x + 11,c.y }, col, 1.6f);break;
	case TitleBtnKind::Minimize: dl->AddLine({ c.x - 6,c.y + 2 }, { c.x + 6,c.y + 2 }, col, 2.0f);break;case TitleBtnKind::Close: dl->AddLine({ c.x - 5,c.y - 5 }, { c.x + 5,c.y + 5 }, col, 1.8f);dl->AddLine({ c.x - 5,c.y + 5 }, { c.x + 5,c.y - 5 }, col, 1.8f);break;
	}
}
static bool TitleButton(const char* id, float sz, TitleBtnKind kind) {
	ImVec2 p = ImGui::GetCursorScreenPos();ImGui::InvisibleButton(id, { sz,sz });bool hov = ImGui::IsItemHovered();bool act = ImGui::IsItemActive();bool clk = ImGui::IsItemClicked();ImDrawList* dl = ImGui::GetWindowDrawList();
	ImU32 bg = hov ? ImGui::GetColorU32(act ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered) : IM_COL32(0, 0, 0, 0);dl->AddRectFilled(p, { p.x + sz,p.y + sz }, bg, 6.0f);
	DrawTitleButtonGlyph(dl, p, sz, kind, kind == TitleBtnKind::Close && hov ? IM_COL32(255, 180, 180, 255) : ImGui::GetColorU32(ImGuiCol_Text));return clk;
}
enum class NavIcon { Home, Mirror, Palette, Lighting, Bloom, AA, Grade, Fog, Dof, Sharp, Outline, Screen, Vision, Weather, Compare, Perf, Engine, AI, Crosshair, Support, Lens, ShadowLab };
static void DrawNavIcon(ImDrawList* dl, ImVec2 c, float s, NavIcon k, ImU32 col) {
	switch (k) {
	case NavIcon::Home: dl->AddTriangleFilled({ c.x,c.y - s }, { c.x - s * 0.9f,c.y + s * 0.1f }, { c.x + s * 0.9f,c.y + s * 0.1f }, col);dl->AddRectFilled({ c.x - s * 0.5f,c.y }, { c.x + s * 0.5f,c.y + s }, col);break;
	case NavIcon::Mirror: dl->AddLine({ c.x - s,c.y - s * 0.2f }, { c.x + s,c.y - s * 0.2f }, col, 1.5f);dl->AddLine({ c.x - s * 0.7f,c.y + s * 0.2f }, { c.x + s * 0.7f,c.y + s * 0.2f }, col, 1.3f);dl->AddLine({ c.x - s * 0.4f,c.y + s * 0.6f }, { c.x + s * 0.4f,c.y + s * 0.6f }, col, 1.1f);break;
	case NavIcon::Palette: dl->AddCircle(c, s * 0.82f, col, 20, 1.5f);dl->AddCircleFilled({ c.x - s * 0.25f,c.y - s * 0.15f }, s * 0.12f, col);dl->AddCircleFilled({ c.x + s * 0.15f,c.y - s * 0.28f }, s * 0.12f, col);dl->AddCircleFilled({ c.x + s * 0.28f,c.y + s * 0.05f }, s * 0.12f, col);break;
	case NavIcon::Lighting: dl->AddCircleFilled(c, s * 0.34f, col);for (int i = 0;i < 8;++i) { float a = i * 3.1415926f / 4.0f;dl->AddLine({ c.x + cosf(a) * s * 0.55f,c.y + sinf(a) * s * 0.55f }, { c.x + cosf(a) * s * 1.0f,c.y + sinf(a) * s * 1.0f }, col, 1.4f); }break;
	case NavIcon::Bloom: dl->AddCircleFilled(c, s * 0.40f, col);dl->AddLine({ c.x - s,c.y }, { c.x + s,c.y }, col, 1.3f);dl->AddLine({ c.x,c.y - s }, { c.x,c.y + s }, col, 1.3f);break;
	case NavIcon::AA: dl->AddRect({ c.x - s,c.y - s * 0.75f }, { c.x + s,c.y + s * 0.75f }, col, 2.0f, 0, 1.4f);dl->AddLine({ c.x - s * 0.5f,c.y - s * 0.75f }, { c.x + s * 0.3f,c.y + s * 0.75f }, col, 1.3f);break;
	case NavIcon::Grade: dl->AddRectFilled({ c.x - s,c.y - s * 0.8f }, { c.x - s * 0.35f,c.y + s }, col);dl->AddRectFilled({ c.x - s * 0.18f,c.y - s * 0.2f }, { c.x + s * 0.18f,c.y + s }, col);dl->AddRectFilled({ c.x + s * 0.35f,c.y - s * 0.55f }, { c.x + s,c.y + s }, col);break;
	case NavIcon::Fog: dl->AddCircle({ c.x,c.y - s * 0.15f }, s * 0.55f, col, 18, 1.4f);dl->AddLine({ c.x - s,c.y + s * 0.6f }, { c.x + s,c.y + s * 0.6f }, col, 1.5f);break;
	case NavIcon::Dof: dl->AddCircle(c, s * 0.85f, col, 18, 1.4f);dl->AddCircle(c, s * 0.45f, col, 14, 1.4f);break;
	case NavIcon::Sharp: dl->AddLine({ c.x + s * 0.15f,c.y - s }, { c.x - s * 0.3f,c.y + s * 0.05f }, col, 1.6f);dl->AddLine({ c.x - s * 0.3f,c.y + s * 0.05f }, { c.x + s * 0.08f,c.y + s * 0.05f }, col, 1.6f);dl->AddLine({ c.x + s * 0.08f,c.y + s * 0.05f }, { c.x - s * 0.1f,c.y + s }, col, 1.6f);break;
	case NavIcon::Outline: dl->AddRect({ c.x - s,c.y - s * 0.8f }, { c.x + s,c.y + s * 0.8f }, col, 2.0f, 0, 1.4f);break;
	case NavIcon::Screen: dl->AddLine({ c.x - s,c.y - s * 0.5f }, { c.x + s,c.y - s * 0.5f }, col, 1.8f);dl->AddLine({ c.x - s,c.y }, { c.x + s,c.y }, col, 1.8f);dl->AddLine({ c.x - s,c.y + s * 0.5f }, { c.x + s,c.y + s * 0.5f }, col, 1.8f);break;
	case NavIcon::Vision: dl->AddCircleFilled({ c.x - s * 0.4f,c.y }, s * 0.30f, col);dl->AddCircleFilled({ c.x + s * 0.4f,c.y }, s * 0.30f, col);dl->AddRectFilled({ c.x - s * 0.4f,c.y - s * 0.2f }, { c.x + s * 0.4f,c.y + s * 0.2f }, col);break;
	case NavIcon::Weather: dl->AddCircleFilled({ c.x,c.y - s * 0.6f }, s * 0.3f, col);dl->AddTriangleFilled({ c.x - s * 0.8f,c.y + s * 0.6f }, { c.x + s * 0.8f,c.y + s * 0.6f }, { c.x,c.y - s * 0.2f }, col);break;
	case NavIcon::Compare: dl->AddRect({ c.x - s,c.y - s * 0.75f }, { c.x + s,c.y + s * 0.75f }, col, 1.8f, 0, 1.2f);dl->AddLine({ c.x,c.y - s * 0.75f }, { c.x,c.y + s * 0.75f }, col, 1.5f);break;
	case NavIcon::Perf: dl->AddRectFilled({ c.x - s * 0.7f,c.y - s * 0.2f }, { c.x - s * 0.3f,c.y + s }, col);dl->AddRectFilled({ c.x - s * 0.15f,c.y - s }, { c.x + s * 0.15f,c.y + s }, col);dl->AddRectFilled({ c.x + s * 0.3f,c.y - s * 0.55f }, { c.x + s * 0.7f,c.y + s }, col);break;
	case NavIcon::Engine: dl->AddCircle(c, s * 0.8f, col, 16, 1.3f);for (int j = 0;j < 6;++j) { float a = j * (3.14159f / 3.0f);dl->AddLine(c, { c.x + cosf(a) * s * 0.75f,c.y + sinf(a) * s * 0.75f }, col, 1.1f); }break;
	case NavIcon::AI: dl->AddCircleFilled(c, s * 0.5f, col);for (int j = 0;j < 8;++j) { float a = j * (3.14159f * 2.0f / 8.0f);dl->AddLine({ c.x + cosf(a) * s * 0.7f,c.y + sinf(a) * s * 0.7f }, { c.x + cosf(a) * s * 1.1f,c.y + sinf(a) * s * 1.1f }, col, 2.0f); }break;
	case NavIcon::Crosshair: dl->AddCircle(c, s * 0.55f, col, 18, 1.4f);dl->AddLine({ c.x - s,c.y }, { c.x - s * 0.35f,c.y }, col, 1.4f);dl->AddLine({ c.x + s * 0.35f,c.y }, { c.x + s,c.y }, col, 1.4f);dl->AddLine({ c.x,c.y - s }, { c.x,c.y - s * 0.35f }, col, 1.4f);dl->AddLine({ c.x,c.y + s * 0.35f }, { c.x,c.y + s }, col, 1.4f);break;
	case NavIcon::Support: dl->AddRect({ c.x - s,c.y - s * 0.6f }, { c.x + s,c.y + s * 0.6f }, col, 2.0f, 0, 1.4f);dl->AddLine({ c.x - s * 0.4f,c.y - s * 0.2f }, { c.x + s * 0.4f,c.y - s * 0.2f }, col, 1.0f);dl->AddLine({ c.x - s * 0.4f,c.y + s * 0.2f }, { c.x + s * 0.4f,c.y + s * 0.2f }, col, 1.0f);break;
	case NavIcon::Lens: dl->AddCircle({ c.x - s * 0.15f,c.y - s * 0.15f }, s * 0.58f, col, 18, 1.5f);dl->AddLine({ c.x + s * 0.20f,c.y + s * 0.20f }, { c.x + s * 0.80f,c.y + s * 0.80f }, col, 1.8f);break;
	case NavIcon::ShadowLab: dl->AddCircle({ c.x,c.y - s * 0.25f }, s * 0.46f, col, 18, 1.3f);dl->AddLine({ c.x - s,c.y + s * 0.55f }, { c.x + s,c.y + s * 0.55f }, col, 1.4f);dl->AddCircleFilled({ c.x + s * 0.35f,c.y - s * 0.10f }, s * 0.18f, col);break;
	}
}
static bool NavItem(const char* label, bool sel, NavIcon iconKind) {
	ImVec2 p0 = ImGui::GetCursorScreenPos();float rowH = ImGui::GetTextLineHeightWithSpacing() + 10.0f;float w = ImGui::GetContentRegionAvail().x;ImGui::PushID(label);bool clicked = ImGui::InvisibleButton("##nr", { w,rowH });bool hov = ImGui::IsItemHovered();ImDrawList* dl = ImGui::GetWindowDrawList();ImVec2 p1 = { p0.x + w,p0.y + rowH };float anim = AnimF(ImGui::GetID("##n"), sel ? 1.f : (hov ? 0.45f : 0.f), 14.f);
	if (sel) { dl->AddRectFilled(p0, p1, IM_COL32(18, 18, 22, 255), 7.0f);dl->AddRectFilled({ p0.x,p0.y + 4 }, { p0.x + 3,p1.y - 4 }, g_accentCol, 2.0f); }
	else if (anim > 0.01f)dl->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, static_cast<ImU32>(anim * 14)), 7.0f);
	float slide = anim * 3.0f;ImVec2 ic = { p0.x + 15 + slide,p0.y + rowH * 0.5f };ImU32 icol = sel ? g_accentCol : hov ? IM_COL32(175, 175, 185, 255) : IM_COL32(110, 110, 120, 255);DrawNavIcon(dl, ic, 5.7f, iconKind, icol);
	ImU32 tcol = sel ? IM_COL32(230, 230, 235, 255) : hov ? IM_COL32(180, 180, 190, 255) : IM_COL32(118, 118, 128, 255);dl->AddText({ p0.x + 29 + slide,p0.y + (rowH - ImGui::GetTextLineHeight()) * 0.5f }, tcol, label);ImGui::PopID();return clicked;
}
static void DrawCrosshairAt(ImDrawList* fg, ImVec2 c, const EffectSettings& fx) {
	int a = static_cast<int>(fx.crosshairOpacity * 255.0f);ImU32 col = IM_COL32(static_cast<int>(fx.crosshairColor[0] * 255), static_cast<int>(fx.crosshairColor[1] * 255), static_cast<int>(fx.crosshairColor[2] * 255), a);ImU32 sh = IM_COL32(0, 0, 0, static_cast<int>(a * 0.7f));
	float s = fx.crosshairSize, g = fx.crosshairGap, t = fx.crosshairThickness;
	auto line = [&](ImVec2 p1, ImVec2 p2) {fg->AddLine({ p1.x + 1,p1.y + 1 }, { p2.x + 1,p2.y + 1 }, sh, t + 1.2f);fg->AddLine(p1, p2, col, t);};
	if (fx.crosshairStyle == 0 || fx.crosshairStyle == 3) { line({ c.x - s - g,c.y }, { c.x - g,c.y });line({ c.x + g,c.y }, { c.x + s + g,c.y });line({ c.x,c.y - s - g }, { c.x,c.y - g });line({ c.x,c.y + g }, { c.x,c.y + s + g }); }
	if (fx.crosshairStyle == 1 || fx.crosshairStyle == 3) { fg->AddCircleFilled({ c.x + 1,c.y + 1 }, t, sh);fg->AddCircleFilled(c, t, col); }
	if (fx.crosshairStyle == 2) { fg->AddCircle({ c.x + 1,c.y + 1 }, s, sh, 24, t + 1.2f);fg->AddCircle(c, s, col, 24, t); }
}
static void DrawDiscordLogo(ImDrawList* dl, ImVec2 center, float size, ImU32 col) {
	float s = size * 0.5f;dl->AddQuadFilled({ center.x - s * 1.0f,center.y - s * 0.6f }, { center.x - s * 0.6f,center.y - s * 1.1f }, { center.x - s * 0.3f,center.y - s * 0.9f }, { center.x - s * 0.5f,center.y - s * 0.3f }, col);
	dl->AddQuadFilled({ center.x + s * 1.0f,center.y - s * 0.6f }, { center.x + s * 0.6f,center.y - s * 1.1f }, { center.x + s * 0.3f,center.y - s * 0.9f }, { center.x + s * 0.5f,center.y - s * 0.3f }, col);
	dl->AddRectFilled({ center.x - s * 0.9f,center.y - s * 0.6f }, { center.x + s * 0.9f,center.y + s * 0.6f }, col, s * 0.4f);dl->AddCircleFilled({ center.x - s * 0.35f,center.y - s * 0.1f }, s * 0.15f, IM_COL32(88, 101, 242, 255));dl->AddCircleFilled({ center.x + s * 0.35f,center.y - s * 0.1f }, s * 0.15f, IM_COL32(88, 101, 242, 255));
}
static void DrawCleanSplash(ImDrawList* dl, ImVec2 wp, ImVec2 ws, float elapsed) {
	float alpha = g_splashAlpha;float intro = std::min(1.0f, g_splashIntro);float introE = EaseOutCubic(intro);
	dl->AddRectFilled(wp, { wp.x + ws.x,wp.y + ws.y }, IM_COL32(8, 8, 12, static_cast<int>(248 * alpha)), 14.0f);dl->AddRect(wp, { wp.x + ws.x,wp.y + ws.y }, IM_COL32(28, 28, 34, static_cast<int>(255 * alpha)), 14.0f, 0, 1.0f);
	float lineT = fmodf(elapsed * 0.9f, 1.0f);float lineW = ws.x * 0.28f;float lineX = wp.x + lineT * (ws.x + lineW) - lineW;dl->AddRectFilledMultiColor({ lineX,wp.y + ws.y - 3.0f }, { lineX + lineW,wp.y + ws.y - 1.0f }, g_accentCol & 0x00FFFFFF, g_accentCol, g_accentCol, g_accentCol & 0x00FFFFFF);
	ImVec2 ctr = { wp.x + ws.x * 0.5f,wp.y + ws.y * 0.37f };float lr = 44.0f * (0.78f + 0.22f * introE);
	if (g_logoSrv && g_logoW > 0)dl->AddImage((ImTextureID)g_logoSrv, { ctr.x - lr,ctr.y - lr }, { ctr.x + lr,ctr.y + lr }, { 0,0 }, { 1,1 }, IM_COL32(255, 255, 255, static_cast<int>(255 * alpha * introE)));else dl->AddCircleFilled(ctr, lr, g_accentCol);
	const char* title = "StudReshader";ImVec2 tsz = ImGui::CalcTextSize(title);dl->AddText({ ctr.x - tsz.x * 0.5f,ctr.y + 62.0f }, IM_COL32(236, 236, 240, static_cast<int>(255 * alpha)), title);
	const char* sub = "Initializing overlay and syncing visual modules";ImVec2 ssz = ImGui::CalcTextSize(sub);dl->AddText({ ctr.x - ssz.x * 0.5f,ctr.y + 82.0f }, IM_COL32(120, 120, 128, static_cast<int>(255 * alpha)), sub);
	float prog = std::min(1.0f, elapsed / 0.85f);float bX = wp.x + 44.0f;float bY = ctr.y + 126.0f;float bW = ws.x - 88.0f;dl->AddRectFilled({ bX,bY }, { bX + bW,bY + 8.0f }, IM_COL32(24, 24, 30, static_cast<int>(255 * alpha)), 4.0f);
	if (prog > 0.0f) { dl->AddRectFilledMultiColor({ bX,bY }, { bX + bW * prog,bY + 8.0f }, g_accentDimCol, g_accentCol, g_accentCol, g_accentDimCol);float sweep = std::clamp(bX + fmodf(elapsed * 160.0f, bW + 30.0f) - 30.0f, bX - 30.0f, bX + bW);dl->AddRectFilledMultiColor({ sweep,bY }, { sweep + 30.0f,bY + 8.0f }, IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 110), IM_COL32(255, 255, 255, 110), IM_COL32(255, 255, 255, 0)); }
}
static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))return true;
	switch (msg) {
	case WM_MOUSEACTIVATE:
		if (g_wantTextInput)return MA_ACTIVATE;
		return MA_NOACTIVATE;
	case WM_NCHITTEST: {
		if (!g_showMenu)return HTTRANSPARENT;
		POINT pt{ GET_X_LPARAM(lp),GET_Y_LPARAM(lp) };
		if (g_panelRect.right > g_panelRect.left && PtInRect(&g_panelRect, pt))return HTCLIENT;
		return HTTRANSPARENT;
	}
	case WM_SIZE:
		if (g_dev && wp != SIZE_MINIMIZED) { DropRTV();g_W = LOWORD(lp);g_H = HIWORD(lp);if (g_sc && g_W > 0 && g_H > 0) { if (g_ctx) { ID3D11RenderTargetView* nullRtv[1] = { nullptr };g_ctx->OMSetRenderTargets(1, nullRtv, nullptr); }g_sc->ResizeBuffers(0, g_W, g_H, DXGI_FORMAT_UNKNOWN, 0);CreateRTV();if (g_dcd)g_dcd->Commit(); } }return 0;
	case WM_SYSCOMMAND: if ((wp & 0xfff0) == SC_KEYMENU)return 0;break;
	case WM_CLOSE: DestroyWindow(hwnd);return 0;
	case WM_DESTROY: PostQuitMessage(0);return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}
struct NavEntry { const char* label;NavIcon icon;const char* keywords; };
static const NavEntry kNav[] = {
{"Quick Setup",NavIcon::Home,"quick setup preset config json save load"},
{"Ground Mirror",NavIcon::Mirror,"ground mirror reflection floor glossy clearcoat"},
{"Reflection Colors",NavIcon::Palette,"reflection colors tint specular neon"},
{"Lighting",NavIcon::Lighting,"lighting exposure temperature vibrance sky sun ambient"},
{"Bloom & Rays",NavIcon::Bloom,"bloom rays godray threshold light"},
{"Anti-Aliasing",NavIcon::AA,"antialiasing fxaa temporal fsr cas sharpness aa"},
{"Color Grading",NavIcon::Grade,"color grading tonemap theme aces reinhard"},
{"Depth Fog",NavIcon::Fog,"depth fog haze distance color"},
{"Depth of Field",NavIcon::Dof,"depth of field dof blur focus tiltshift"},
{"Sharpening",NavIcon::Sharp,"sharpening radius motion blur"},
{"Cel Outlines",NavIcon::Outline,"cel outlines edge thickness color"},
{"Screen Effects",NavIcon::Screen,"screen effects chromatic vignette grain scanline crt glitch"},
{"Vision Modes",NavIcon::Vision,"vision night thermal"},
{"Weather & Waves",NavIcon::Weather,"weather waves rain wind distortion snow"},
{"Split Compare",NavIcon::Compare,"split compare divider before after"},
{"Crosshair",NavIcon::Crosshair,"crosshair dot circle gap size"},
{"Performance",NavIcon::Perf,"performance fps gpu vram capture"},
{"Engine",NavIcon::Engine,"engine depth ai gpu"},
{"AI Upscale",NavIcon::AI,"ai upscale realesrgan quicksr superresolution"},
{"Support",NavIcon::Support,"support discord link"},
{"Prism Split",NavIcon::Lens,"prism split chromatic rgb"},
{"Halftone Studio",NavIcon::Screen,"halftone dot comic print"},
{"Pixel Lab",NavIcon::Screen,"pixel lab block retro"},
{"Poster Lab",NavIcon::Grade,"poster lab levels quantize"},
{"Embossed Film",NavIcon::Outline,"emboss film relief"},
{"Duotone Wash",NavIcon::Palette,"duotone wash warm cool"},
{"Light Streaks",NavIcon::Bloom,"light streaks anamorphic"},
{"VHS Deck",NavIcon::Screen,"vhs deck tape jitter color crawl"},
{"Scan Pulse",NavIcon::Screen,"scan pulse band"},
{"Heat Haze",NavIcon::Weather,"heat haze distortion shimmer"},
{"Edge Glow",NavIcon::Outline,"edge glow luminous"},
{"Color Isolate",NavIcon::Palette,"color isolate hue range"},
{"Luma Fade",NavIcon::Lighting,"luma fade brightness tint"},
{"Crystal Mosaic",NavIcon::Lens,"crystal mosaic glass block"},
{"Radial Tint",NavIcon::Palette,"radial tint gradient vignette"},
{"Shadow Crush",NavIcon::ShadowLab,"shadow crush dark pivot"},
{"Highlight Softener",NavIcon::Bloom,"highlight softener bloom threshold"},
{"Tunnel Vision",NavIcon::Vision,"tunnel vision feather radius"},
{"Corner Warp",NavIcon::Lens,"corner warp lens curve"},
};
static constexpr int kNavCount = sizeof(kNav) / sizeof(kNav[0]);
static void SdApplyToScreen(AppConfig& cfg, CompositingPipeline& comp) {
	std::lock_guard<std::mutex> lk(g_genMtx);
	if (g_generatedW <= 0 || g_generatedRgba.empty())return;
	comp.UpdateStyleTex(g_dev, g_ctx, g_generatedRgba, g_generatedW, g_generatedH);
	cfg.fx.upscaleSD = true;
	g_sdApplied = true;
	SaveConfig(cfg);
	g_sdStatus = "Applied to screen | overlay is LIVE. Click 'Stop Applying' to remove it.";
}
static void SdStopApplying(AppConfig& cfg) {
	cfg.fx.upscaleSD = false;
	g_sdApplied = false;
	SaveConfig(cfg);
	g_sdStatus = "Overlay stopped | your screen is back to normal.";
}
static void SdSavePng() {
	std::lock_guard<std::mutex> lk(g_genMtx);
	if (g_generatedW <= 0 || g_generatedRgba.empty()) { g_sdStatus = "Nothing to save | generate an image first.";return; }
	wchar_t exePath[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	std::wstring exeDir(exePath);
	size_t slash = exeDir.find_last_of(L"\\/");
	exeDir = (slash != std::wstring::npos) ? exeDir.substr(0, slash + 1) : L"";
	auto now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm tmv{};
	localtime_s(&tmv, &tt);
	wchar_t fname[128];
	swprintf_s(fname, L"studai_%04d%02d%02d_%02d%02d%02d.png", tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday, tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
	std::wstring path = exeDir + fname;
	if (SaveRgbaAsPngWIC(path, g_generatedRgba, g_generatedW, g_generatedH)) {
		g_sdStatus = "Saved PNG: " + WtoA(fname);
	}
	else {
		g_sdStatus = "Failed to save PNG.";
	}
}
#if !defined(_MSC_VER)
struct EXCEPTION_POINTERS;
#ifndef EXCEPTION_CONTINUE_SEARCH
#define EXCEPTION_CONTINUE_SEARCH 0
#endif
#endif
static LONG WINAPI SrCrashFilter(EXCEPTION_POINTERS* ep) {
#if defined(_MSC_VER)
	try {
		std::ofstream f("models\\crash.log", std::ios::out);
		if (f) {
			f << "code 0x" << std::hex << (unsigned)ep->ExceptionRecord->ExceptionCode << std::dec << "\n";
			f << "addr 0x" << std::hex << (uintptr_t)ep->ExceptionRecord->ExceptionAddress << std::dec << "\n";
			HMODULE hm = nullptr;
			GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCWSTR)ep->ExceptionRecord->ExceptionAddress, &hm);
			if (hm) {
				wchar_t mp[MAX_PATH] = {};
				GetModuleFileNameW(hm, mp, MAX_PATH);
				f << "module " << WtoA(mp) << "\n";
			}
			f << "flags 0x" << std::hex << (unsigned)ep->ExceptionRecord->ExceptionFlags << std::dec << "\n";
		}
	}
	catch (...) {}
#else
	(void)ep;
	try { std::ofstream f("models\\crash.log", std::ios::out);if (f)f << "crash\n"; }
	catch (...) {}
#endif
	return EXCEPTION_CONTINUE_SEARCH;
}
int APIENTRY wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	(void)hPrevInstance;(void)lpCmdLine;(void)nCmdShow;
#if defined(_MSC_VER)
	SetUnhandledExceptionFilter(SrCrashFilter);
#endif
	EnablePerMonitorDpiAwareness();
	timeBeginPeriod(1);
	{
		wchar_t exePath[MAX_PATH] = {};
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		wchar_t* lastSlash = wcsrchr(exePath, L'\\');
		if (lastSlash) { *lastSlash = 0;SetCurrentDirectoryW(exePath); }
	}
	g_singleInstanceMutex = CreateMutexW(nullptr, FALSE, kMutexName);
	if (!g_singleInstanceMutex || GetLastError() == ERROR_ALREADY_EXISTS) { ShowNativeError(L"StudReshader", L"StudReshader is already open.");if (g_singleInstanceMutex)CloseHandle(g_singleInstanceMutex);return 0; }
	g_robloxHwnd = FindRobloxWindow();RefreshRobloxPid();
	if (!g_robloxHwnd) {
		for (int tries = 0;tries < 50 && !g_robloxHwnd;++tries) {
			Sleep(100);
			g_robloxHwnd = FindRobloxWindow();RefreshRobloxPid();
		}
		if (!g_robloxHwnd) { ShowNativeError(L"StudReshader", L"Could not find a Roblox window. Make sure Roblox is open(or starting up)and try again.");CloseHandle(g_singleInstanceMutex);return 0; }
	}
	LoadAppIcons();
	WNDCLASSEXW wc{};wc.cbSize = sizeof(wc);wc.style = CS_CLASSDC;wc.lpfnWndProc = WndProc;wc.hInstance = hInst;wc.hIcon = g_appIcon ? g_appIcon : LoadIconW(nullptr, IDI_APPLICATION);wc.hIconSm = g_appIconSm ? g_appIconSm : LoadIconW(nullptr, IDI_APPLICATION);wc.lpszClassName = kWindowClass;RegisterClassExW(&wc);
	RECT rbxRect{};if (!GetWindowRect(g_robloxHwnd, &rbxRect)) { ShowNativeError(L"StudReshader", L"Failed to read the Roblox window position.");UnregisterClassW(kWindowClass, hInst);CloseHandle(g_singleInstanceMutex);return 0; }
	int initW = std::max(100, static_cast<int>(rbxRect.right - rbxRect.left));int initH = std::max(100, static_cast<int>(rbxRect.bottom - rbxRect.top));
	g_overlayHwnd = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_APPWINDOW | WS_EX_NOACTIVATE, kWindowClass, L"StudReshader 1.0 Beta", WS_POPUP, rbxRect.left, rbxRect.top, initW, initH, nullptr, nullptr, hInst, nullptr);
	g_targetMonitor = MonitorFromWindow(g_robloxHwnd, MONITOR_DEFAULTTONEAREST);
	SetWindowDisplayAffinity(g_overlayHwnd, WDA_EXCLUDEFROMCAPTURE);
	if (!CreateDeviceD3D(g_overlayHwnd, initW, initH)) { CleanupDeviceD3D();DestroyWindow(g_overlayHwnd);UnregisterClassW(kWindowClass, hInst);CloseHandle(g_singleInstanceMutex);return 1; }
	if (g_appIcon) { HICON bigCopy = CopyIcon(g_appIcon);HICON smCopy = g_appIconSm ? CopyIcon(g_appIconSm) : CopyIcon(g_appIcon);if (bigCopy)SendMessageW(g_overlayHwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(bigCopy));if (smCopy)SendMessageW(g_overlayHwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(smCopy));SetWindowPos(g_overlayHwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED); }
	SetOverlayVisible(true);UpdateWindow(g_overlayHwnd);SetForegroundWindow(g_robloxHwnd);
	IMGUI_CHECKVERSION();ImGui::CreateContext();ImGuiIO& io = ImGui::GetIO();io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef ImGuiConfigFlags_DockingEnable
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
	io.IniFilename = nullptr;float dpiScale = 1.0f; { HMODULE hu = GetModuleHandleW(L"user32.dll");typedef UINT(WINAPI* GetDpiForWindowFn)(HWND);GetDpiForWindowFn getDpi = hu ? (GetDpiForWindowFn)GetProcAddress(hu, "GetDpiForWindow") : nullptr;if (getDpi && g_overlayHwnd) { UINT d = getDpi(g_overlayHwnd);if (d >= 96)dpiScale = static_cast<float>(d) / 96.0f; } }
	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f * dpiScale);if (!font)io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f * dpiScale);
	ApplyGuiTheme(g_guiTheme);ImGui_ImplWin32_Init(g_overlayHwnd);ImGui_ImplDX11_Init(g_dev, g_ctx);LoadLogoTexture(g_dev);
	{ std::thread([] {std::wstring cachePath = std::filesystem::current_path().wstring() + L"\\discord_public_icon.ico";if (!std::filesystem::exists(cachePath)) { std::atomic<double> prog{ 0.0 };std::string err;DownloadUrlToFile(L"https://discord.com/assets/847541504914fd33810e70a0ea73177e.ico", cachePath, &prog, &err); }}).detach(); }
	LoadDiscordTexture(g_dev);
	auto gpus = EnumerateGpus();static AppConfig cfg;LoadConfig(cfg);static PerfTracker perf;static DepthEngine depth;g_depthEnginePtr = &depth;static ScreenCapture cap;static CompositingPipeline comp;static GenericProvisioner mp;static UnifiedAiEngine aiEngine;static GenericProvisioner aiProv[1];
	bool aiInit = false;bool depthInit = false;bool done = false;bool captureOk = false;
	if (!comp.Init(g_dev)) { std::string serr = comp.LastError();if (serr.empty())serr = "Unknown shader compilation failure.";MessageBoxA(nullptr, serr.c_str(), "StudReshader | Shader Compile Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST); }
	comp.GenFallbackDepth(g_dev, g_ctx);captureOk = InitCaptureForRobloxWindow(cap, g_dxgiAdapter.Get(), g_dev, g_robloxHwnd);
	if (SdFilesReady()) {
		g_sdGenState = SdGenState::Ready;
		g_sdStatus = "Model ready | press Run to generate.";
	}
	ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();DrawCleanSplash(ImGui::GetForegroundDrawList(), { 0,0 }, { 380,240 }, 0.0f);ImGui::Render();
	{ std::lock_guard<std::mutex> ctxLk(g_ctxMtx);g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);const float clearFirst[4] = { 0,0,0,0 };g_ctx->ClearRenderTargetView(g_rtv, clearFirst);ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());g_sc->Present(0, 0);g_dcd->Commit(); }
	auto splashT0 = std::chrono::steady_clock::now();
	while (!done) {
		auto loopT0 = std::chrono::steady_clock::now();
		MSG msg;while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg);DispatchMessage(&msg);if (msg.message == WM_QUIT)done = true; }
		if (done || !g_appRunning)break;if (GetAsyncKeyState(VK_END) & 0x8000)break;
		static bool insP = false;
		if (GetAsyncKeyState(VK_INSERT) & 0x8000) { if (!insP) { g_showMenu = !g_showMenu;insP = true;if (g_showMenu)SetForegroundWindow(g_overlayHwnd);else { g_showCloseToast = true;g_closeToastTimer = 3.0f;SetForegroundWindow(g_robloxHwnd); } } }
		else insP = false;
		HWND fg = GetForegroundWindow();bool overlayFocused = (fg == g_overlayHwnd);bool robloxAppForeground = IsRobloxProcessWindow(fg);
		if (!IsWindow(g_robloxHwnd)) { g_robloxHwnd = FindRobloxWindow();RefreshRobloxPid();if (!IsWindow(g_robloxHwnd)) { if (!g_runtimeLossHandled) { g_runtimeLossHandled = true;ShowNativeError(L"StudReshader", L"Roblox was closed. StudReshader will now exit."); }break; } }
		if (++g_retargetCounter >= 180) { g_retargetCounter = 0;HWND better = FindRobloxWindow();if (better && better != g_robloxHwnd && IsWindow(better)) { g_robloxHwnd = better;RefreshRobloxPid(); } }
		float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - g_startTime).count();
		float splashElapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - splashT0).count();
		bool startupSplash = g_showSplash || splashElapsed < 1.5f;
		bool wantHidden = false;if (IsIconic(g_robloxHwnd))wantHidden = true;bool allowOverlayVisible = startupSplash || !wantHidden;
		SetOverlayVisible(allowOverlayVisible);
		POINT curMouse{};GetCursorPos(&curMouse);
		g_mouseInPanel = g_showMenu && !g_showSplash && g_panelRect.right > g_panelRect.left && PtInRect(&g_panelRect, curMouse);
		g_wantTextInput = io.WantTextInput;
		LONG_PTR exStyle = GetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE);LONG_PTR desired = exStyle;
		if (!g_showMenu || g_showSplash) {
			desired |= WS_EX_TRANSPARENT;
			desired |= WS_EX_NOACTIVATE;
		}
		else {
			if (g_mouseInPanel) {
				desired &= ~WS_EX_TRANSPARENT;
				if (g_wantTextInput) { desired &= ~WS_EX_NOACTIVATE; }
				else { desired |= WS_EX_NOACTIVATE; }
			}
			else {
				desired |= WS_EX_TRANSPARENT;
				desired |= WS_EX_NOACTIVATE;
			}
		}
		if (desired != exStyle) { SetWindowLongPtr(g_overlayHwnd, GWL_EXSTYLE, desired);SetWindowPos(g_overlayHwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED); }
		perf.PushTick();PerfSample psNow = perf.Snap();g_fpsHist[g_fpsPos] = static_cast<float>(psNow.overlayFps);g_fpsPos = (g_fpsPos + 1) % 160;
		if (cfg.autoTrackWindow) {
			RECT cr{};if (GetWindowRect(g_robloxHwnd, &cr)) {
				int tw = cr.right - cr.left;int th = cr.bottom - cr.top;if (IsIconic(g_robloxHwnd))SetOverlayVisible(false);else if (tw > 100 && th > 100) {
					bool moved = cr.left != rbxRect.left || cr.top != rbxRect.top || tw != (rbxRect.right - rbxRect.left) || th != (rbxRect.bottom - rbxRect.top);
					if (moved) { rbxRect = cr;SetWindowPos(g_overlayHwnd, HWND_TOPMOST, rbxRect.left, rbxRect.top, tw, th, SWP_NOACTIVATE); }
					HMONITOR currentMon = MonitorFromWindow(g_robloxHwnd, MONITOR_DEFAULTTONEAREST);if (!cap.Valid() || currentMon != g_captureMonitor)captureOk = InitCaptureForRobloxWindow(cap, g_dxgiAdapter.Get(), g_dev, g_robloxHwnd);
					SetOverlayVisible(allowOverlayVisible);
				}
			}
		}
		if (!cap.Valid() && (++g_capRetryCounter >= 120)) { g_capRetryCounter = 0;captureOk = InitCaptureForRobloxWindow(cap, g_dxgiAdapter.Get(), g_dev, g_robloxHwnd); }
		bool needWork = g_showMenu || g_showSplash || g_showCloseToast || cfg.fx.crosshairEnabled ||
			cfg.fx.glossyEnabled || cfg.fx.upscaleSD || cfg.fx.upscaleNAFNet || cfg.fx.splitScreenEnabled ||
			cfg.fx.outlineEnabled || cfg.fx.fogEnabled || cfg.fx.dofEnabled || cfg.fx.ssaoEnabled ||
			cfg.fx.upscaleFSR || cfg.fx.upscaleDLSS || cfg.fx.fxaaEnabled ||
			cfg.fx.bloomIntensity > 0.001f || cfg.fx.vignetteIntensity > 0.001f || cfg.fx.filmGrainAmount > 0.001f ||
			cfg.fx.chromaticAberration > 0.001f || cfg.fx.neonGlowIntensity > 0.001f || cfg.fx.skyGlowStrength > 0.001f ||
			cfg.fx.rainDrops > 0.001f || cfg.fx.snowAmount > 0.001f || cfg.fx.thermalVision > 0.001f ||
			cfg.fx.nightVision > 0.001f || cfg.fx.scanlineIntensity > 0.001f || cfg.fx.crtCurve > 0.001f ||
			cfg.fx.glitchAmount > 0.001f || cfg.fx.waveDistortionAmount > 0.001f || cfg.fx.motionBlurAmount > 0.01f ||
			cfg.fx.godRayIntensity > 0.005f || cfg.fx.contactShadow > 0.001f || g_gpuDrawMode || g_visionEnabled || g_liveEnabled || g_radarOn;
		g_capActive.store(needWork);
		g_capPixelsNeeded.store(g_liveEnabled || g_visionEnabled || g_radarOn || g_gpuDrawMode || cfg.fx.upscaleSD || cfg.fx.upscaleNAFNet ||
			cfg.fx.glossyEnabled || cfg.fx.splitScreenEnabled || cfg.fx.outlineEnabled || cfg.fx.fogEnabled ||
			cfg.fx.dofEnabled || cfg.fx.ssaoEnabled || cfg.fx.upscaleFSR || cfg.fx.upscaleDLSS || cfg.fx.fxaaEnabled ||
			cfg.fx.bloomIntensity > 0.001f || cfg.fx.vignetteIntensity > 0.001f || cfg.fx.filmGrainAmount > 0.001f ||
			cfg.fx.chromaticAberration > 0.001f || cfg.fx.neonGlowIntensity > 0.001f || cfg.fx.skyGlowStrength > 0.001f ||
			cfg.fx.rainDrops > 0.001f || cfg.fx.snowAmount > 0.001f || cfg.fx.thermalVision > 0.001f ||
			cfg.fx.nightVision > 0.001f || cfg.fx.scanlineIntensity > 0.001f || cfg.fx.crtCurve > 0.001f ||
			cfg.fx.glitchAmount > 0.001f || cfg.fx.waveDistortionAmount > 0.001f || cfg.fx.motionBlurAmount > 0.01f ||
			cfg.fx.godRayIntensity > 0.005f || cfg.fx.contactShadow > 0.001f);
		if (!needWork && g_overlayVisible)SetOverlayVisible(false);
		if (needWork && !g_overlayVisible && allowOverlayVisible)SetOverlayVisible(true);
		if (!startupSplash && !IsIconic(g_robloxHwnd)) {
#if SR_HAS_ONNX
			if (cfg.fx.engineMode == 1) {
				try {
					auto st = mp.State();if (st == ProvState::Checking) { mp.Begin({ L"https://huggingface.co/onnx-community/depth-anything-v2-small/resolve/main/onnx/model.onnx" }, L"models\\vision\\depth_anything_v2_vits.onnx", 90000000); }
					else if (st == ProvState::Downloading)g_aiStatus = "Downloading depth model " + std::to_string(static_cast<int>(mp.Prog() * 100.0)) + "%";
					else if (st == ProvState::Verifying)g_aiStatus = "Verifying depth model";
					else if (st == ProvState::Failed) { g_aiStatus = "Depth model failed | reverting to GPU mode";cfg.fx.engineMode = 0;depthInit = false; }
					else if (st == ProvState::Ready && !depthInit) { if (depth.Init(224, mp.Path(), g_ctx)) { depthInit = true;g_aiStatus = "AI depth active(safe mode)"; } else { cfg.fx.engineMode = 0;depthInit = false;g_aiStatus = "Depth engine failed | reverting"; } }
				}
				catch (...) { cfg.fx.engineMode = 0;depthInit = false;g_aiStatus = "Unexpected AI error | reverting"; }
			}
			else { depthInit = false;g_aiStatus = "GPU Mode Ready"; }
#else
			cfg.fx.engineMode = 0;g_aiStatus = "AI disabled in this build";
#endif
#if SR_HAS_ONNX
			if (cfg.fx.upscaleNAFNet && !aiInit) { const auto& aiDef = kAiModels[g_selModel];if (aiProv[0].State() == ProvState::Ready) { try { aiInit = aiEngine.Init(aiDef, g_dev, g_ctx); } catch (...) { aiInit = false; } } }
#else
			cfg.fx.upscaleNAFNet = false;
#endif
			if (g_sdGenState == SdGenState::Downloading) {
				SdProvState ps = g_sdProv.State();
				if (ps == SdProvState::Ready) {
					g_sdGenState = SdGenState::Ready;
					g_sdStatus = "Model ready | press Run to generate.";
				}
				else if (ps == SdProvState::Failed) {
					g_sdGenState = SdGenState::DownloadFailed;
					g_sdStatus = "Download failed: " + g_sdProv.Err();
				}
			}
			if (g_sdGenState == SdGenState::Done) {
				std::vector<uint8_t> res;
				int rw = 0, rh = 0;
				bool got = false;
				{
					std::lock_guard<std::mutex> lk(g_resultMtx);
					if (g_resultReady) {
						res.swap(g_resultRgba);
						rw = g_resultW;rh = g_resultH;
						g_resultReady = false;
						got = true;
					}
				}
				if (got) {
					UpdateGeneratedTexture(g_dev, g_ctx, res, rw, rh);
					if (g_sdAutoApply || g_sdContinuous)SdApplyToScreen(cfg, comp);
				}
				g_sdGenState = SdGenState::Ready;
			}
			if (g_sdGenState == SdGenState::Failed) {
				cfg.fx.upscaleSD = false;
				g_sdApplied = false;
				g_sdGenState = SdGenState::Ready;
			}
			{
				HRESULT devReason = S_OK;
				if (g_dev)devReason = g_dev->GetDeviceRemovedReason();
				if (devReason != S_OK) {
					if (g_liveRun.load()) { g_liveRun.store(false);g_sdCancel.store(true); }
					if (g_visionRun.load()) { g_visionRun.store(false); }
					if (g_sdThread.joinable())g_sdThread.detach();
					if (g_visionThread.joinable())g_visionThread.detach();
					g_liveEnabled = false;
					g_visionEnabled = false;
					g_gpuDrawMode = false;
					ResetVisionFx(cfg);
					cfg.fx.upscaleSD = false;
					g_sdApplied = false;
					g_gpuDied = true;
					CheckGpuInfo();
					g_sdStatus = "GPU device removed | AI stopped | restart the app";
					std::string gtx2 = GpuWarningText();
					if (!gtx2.empty())g_sdStatus += " | " + gtx2;
				}
			}
			if (g_liveEnabled && !g_liveRun.load()) {
				g_liveEnabled = false;
				WaitLiveThread(30);
				if (g_liveThread.joinable() && g_liveThreadDone.load())g_liveThread.join();
				cfg.fx.upscaleSD = false;
				g_sdApplied = false;
			}
			if (g_visionEnabled && !g_visionRun.load()) {
				g_visionEnabled = false;
				ResetVisionFx(cfg);
				if (g_visionThread.joinable())g_visionThread.detach();
			}
			if (g_visionEnabled && g_visionRun.load()) {
				double nowS = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
				if (nowS - g_lastVisionBeat.load() > 30.0) {
					g_visionRun.store(false);
					if (g_visionThread.joinable())g_visionThread.detach();
					g_visionEnabled = false;
					ResetVisionFx(cfg);
					g_sdStatus = "AI engine stopped | timeout";
				}
			}
			if (g_brainThinking.load()) {
				double nowS = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
				if (nowS - g_brainThinkStart > 180.0) {
					g_brainThinking.store(false);
					g_brainRun.store(false);
					g_brainCancel.store(true);
					if (g_brainThread.joinable() && g_brainThreadDone.load())g_brainThread.join();
					SetDraft("");
					SetStatus("Assistant timed out | CPU is slow | fix CUDA versions for speed");
					BrainAddMsg("assistant", "(timed out)");
				}
			}
			if (g_liveEnabled && g_liveRun.load()) {
				double nowS = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
				double hwLimit = 30.0;
				if (g_liveModel == 2 && !g_mbEngine.OnGpu())hwLimit = 180.0;
				else if (!g_sdPipeline.FastOnGpu())hwLimit = 60.0;
				double lastBeat = g_lastWorkerBeat.load();
				if (lastBeat > 1.0 && nowS - lastBeat > hwLimit) {
					g_liveRun.store(false);
					g_sdCancel.store(true);
					cfg.fx.upscaleSD = false;
					g_sdApplied = false;
					g_liveEnabled = false;
					g_sdStatus = "Engine stopped | timeout";
				}
			}
			if (!g_liveEnabled && g_liveRun.load())LiveStop(cfg);
			{
				bool capGot = false;
				if (needWork) {
					std::lock_guard<std::mutex> ctxLk(g_ctxMtx);
					capGot = cap.TryGetFrame(g_dev, g_ctx, comp, depth, rbxRect);
				}
				(void)capGot;
				if (g_liveEnabled && g_liveModel == 4)ObjMainLoopTick();
				if (g_liveEnabled && g_liveModel != 4)LiveApplyResult(cfg, comp);
				if (g_visionEnabled) {
					{
						std::lock_guard<std::mutex> lk(g_visionMtx);
						g_activeStyle = g_visionStyle;
					}
					ApplyVisionFx(cfg, g_activeStyle);
					if (g_maskPubDirty.exchange(false)) {
						std::lock_guard<std::mutex> lk(g_maskPubMtx);
						if (!g_maskPub.empty() && g_maskPubW > 0)comp.UpdateMaskTex(g_dev, g_ctx, g_maskPub, g_maskPubW, g_maskPubH);
						else comp.ClearMask(g_ctx);
					}
				}
#if SR_HAS_ONNX
				if (cfg.fx.engineMode == 1 && depthInit) { std::vector<float> dmap;double aiMs = 0.0;if (depth.TryGetResult(dmap, aiMs)) { perf.PushAiMs(aiMs);comp.UpdateDepthTex(g_dev, g_ctx, dmap, depth.Size()); } }
#endif
				if (cfg.fx.upscaleNAFNet) {
#if SR_HAS_ONNX
					if (aiInit && comp.GetScene()) {
						aiEngine.SetStrength(cfg.fx.upscaleStrength);aiEngine.SetQuality(cfg.fx.upscaleQuality);aiEngine.SetInterval(500);aiEngine.Submit(g_dev, g_ctx, comp.GetScene());
						std::vector<uint8_t> outImg;int ow = 0, oh = 0;double ms = 0.0;if (aiEngine.TryGetResult(outImg, ow, oh, ms)) { perf.PushAiMs(ms);comp.UpdateStyleTex(g_dev, g_ctx, outImg, ow, oh); }
					}
#else
					cfg.fx.upscaleNAFNet = false;
#endif
				}
			}
		}
		elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - g_startTime).count();
		if (g_showSplash) { g_splashIntro = std::min(1.0f, g_splashIntro + io.DeltaTime * 2.8f);if (splashElapsed < 1.5f)g_splashAlpha = 1.0f;else { g_showSplash = false;g_splashAlpha = 0.0f;SetOverlayVisible(true);if (IsWindow(g_robloxHwnd))SetForegroundWindow(g_robloxHwnd); } }
		if (g_linkCopied) { g_linkCopiedTimer -= io.DeltaTime;if (g_linkCopiedTimer <= 0.0f)g_linkCopied = false; }
		if (g_showCloseToast) { g_closeToastTimer -= io.DeltaTime;if (g_closeToastTimer <= 0.0f)g_showCloseToast = false; }
		if (g_prevSec != g_activeSec) { g_secAlpha = 0.0f;g_prevSec = g_activeSec; }g_secAlpha = std::min(1.0f, g_secAlpha + io.DeltaTime * 8.0f);
		ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
		if (cfg.fx.crosshairEnabled && !g_showSplash)DrawCrosshairAt(ImGui::GetForegroundDrawList(), { io.DisplaySize.x * 0.5f,io.DisplaySize.y * 0.5f }, cfg.fx);
		if (g_showCloseToast) {
			ImGui::SetNextWindowPos({ io.DisplaySize.x - 360,io.DisplaySize.y - 96 }, ImGuiCond_Always);ImGui::SetNextWindowSize({ 330,66 });ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);ImGui::Begin("##toast", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
			ImDrawList* tdl = ImGui::GetWindowDrawList();ImVec2 twp = ImGui::GetWindowPos(), tws = ImGui::GetWindowSize();tdl->AddRectFilled(twp, { twp.x + tws.x,twp.y + tws.y }, IM_COL32(18, 18, 24, 245), 10.0f);tdl->AddRectFilled({ twp.x,twp.y }, { twp.x + 6,twp.y + tws.y }, g_accentCol, 10.0f);
			tdl->AddRect(twp, { twp.x + tws.x,twp.y + tws.y }, IM_COL32(255, 170, 80, 255), 10.0f, 0, 1.0f);tdl->AddText({ twp.x + 20,twp.y + 12 }, IM_COL32(255, 255, 255, 255), "MENU HIDDEN");tdl->AddText({ twp.x + 20,twp.y + 34 }, IM_COL32(180, 180, 186, 255), "Press INSERT to open it again.");
			tdl->AddRectFilled({ twp.x,twp.y + tws.y - 3 }, { twp.x + tws.x * (g_closeToastTimer / 3.0f),twp.y + tws.y }, g_accentCol, 10.0f, ImDrawFlags_RoundCornersBottom);ImGui::End();ImGui::PopStyleVar();
		}
		if (g_showSplash) {
			ImGui::SetNextWindowPos({ io.DisplaySize.x * 0.5f,io.DisplaySize.y * 0.5f }, ImGuiCond_Always, { 0.5f,0.5f });ImGui::SetNextWindowSize({ 380,240 }, ImGuiCond_Always);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14);
			ImGui::Begin("##spl", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings);DrawCleanSplash(ImGui::GetWindowDrawList(), ImGui::GetWindowPos(), ImGui::GetWindowSize(), elapsed);ImGui::End();ImGui::PopStyleVar(2);
		}
		else if (g_showMenu) {
			const float titleH = 48.0f;const float footerH = 42.0f;const float sidebarW = g_drawerOpen ? 230.0f : 0.0f;
			ImGui::SetNextWindowPos({ 30,30 }, ImGuiCond_FirstUseEver);
			if (g_compactMode)ImGui::SetNextWindowSize({ std::max(g_lastExpandedMenuSize.x,700.0f),titleH + footerH + 8.0f }, ImGuiCond_Always);
			else { ImGui::SetNextWindowSize({ 980,700 }, ImGuiCond_FirstUseEver);ImGui::SetNextWindowSizeConstraints({ 760,520 }, { 1400,980 }); }
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14);ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.01f,0.01f,0.02f,1 });
			bool menuOpen = true;ImGui::Begin("##SR", &menuOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::SetWindowFontScale(std::clamp(cfg.fx.uiScale, 0.8f, 1.4f));
			if (!menuOpen) { g_showMenu = false;g_showCloseToast = true;g_closeToastTimer = 3.0f;SetForegroundWindow(g_robloxHwnd); }
			ImVec2 wP = ImGui::GetWindowPos();ImVec2 wS = ImGui::GetWindowSize();ImGui::SetCursorPos({ 0,0 });ImGui::InvisibleButton("##drag", { wS.x - 126,titleH });
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) { ImVec2 d = io.MouseDelta;ImGui::SetWindowPos({ wP.x + d.x,wP.y + d.y });wP = ImGui::GetWindowPos(); }
			wS = ImGui::GetWindowSize();if (!g_compactMode)g_lastExpandedMenuSize = wS;ImDrawList* wdl = ImGui::GetWindowDrawList();
			wdl->AddRectFilled(wP, { wP.x + wS.x,wP.y + titleH }, IM_COL32(3, 3, 5, 255), 14.0f, ImDrawFlags_RoundCornersTop);wdl->AddLine({ wP.x,wP.y + titleH }, { wP.x + wS.x,wP.y + titleH }, IM_COL32(16, 16, 20, 255), 1.0f);
			float lX = wP.x + 14.0f;float lY = wP.y + (titleH - 30.0f) * 0.5f;if (g_logoSrv && g_logoW > 0)wdl->AddImage((ImTextureID)g_logoSrv, { lX,lY }, { lX + 30,lY + 30 }, { 0,0 }, { 1,1 });else wdl->AddCircleFilled({ lX + 15,lY + 15 }, 15.0f, g_accentCol);
			wdl->AddText({ lX + 40,lY + 1 }, IM_COL32(235, 235, 240, 255), "StudReshader");wdl->AddText({ lX + 40,lY + 18 }, IM_COL32(110, 110, 118, 255), "1.0 Beta");
			const char* modeStr = cfg.fx.engineMode == 1 ? "AI Depth" : "GPU Mode";ImVec2 msz = ImGui::CalcTextSize(modeStr);float pillW = msz.x + 34.0f, pillH = 24.0f;ImVec2 pillP = { wP.x + 230.0f,wP.y + (titleH - pillH) * 0.5f };
			wdl->AddRectFilled(pillP, { pillP.x + pillW,pillP.y + pillH }, cfg.fx.engineMode == 1 ? (g_accentDimCol & 0x00FFFFFF) | 0xFF000000 : IM_COL32(10, 10, 15, 255), 12.0f);wdl->AddRect(pillP, { pillP.x + pillW,pillP.y + pillH }, g_accentDimCol, 12.0f, 0, 1.0f);
			wdl->AddCircleFilled({ pillP.x + 12,pillP.y + pillH * 0.5f }, 4.0f, cfg.fx.engineMode == 1 ? g_accentCol : g_accentDimCol);wdl->AddText({ pillP.x + 22,pillP.y + (pillH - msz.y) * 0.5f }, g_accentCol, modeStr);
			ImGui::SetCursorPos({ wS.x - 120,(titleH - 26) * 0.5f });if (TitleButton("##drawer", 26, TitleBtnKind::Menu))g_drawerOpen = !g_drawerOpen;
			ImGui::SetCursorPos({ wS.x - 90,(titleH - 26) * 0.5f });if (TitleButton("##theme", 26, TitleBtnKind::Theme)) { g_guiTheme = (g_guiTheme + 1) % 4;ApplyGuiTheme(g_guiTheme); }
			ImGui::SetCursorPos({ wS.x - 60,(titleH - 26) * 0.5f });if (TitleButton("##min", 26, TitleBtnKind::Minimize)) { g_compactMode = !g_compactMode;if (!g_compactMode)SetForegroundWindow(g_overlayHwnd); }
			ImGui::SetCursorPos({ wS.x - 30,(titleH - 26) * 0.5f });if (TitleButton("##close", 26, TitleBtnKind::Close)) { g_showMenu = false;g_showCloseToast = true;g_closeToastTimer = 3.0f;SetForegroundWindow(g_robloxHwnd); }
			float bodyY = titleH;float bodyH = std::max(0.0f, wS.y - titleH - footerH);float contentX = sidebarW;float contentW = wS.x - sidebarW;
			if (!g_compactMode && g_drawerOpen) {
				wdl->AddRectFilled({ wP.x,wP.y + bodyY }, { wP.x + sidebarW,wP.y + bodyY + bodyH }, IM_COL32(4, 4, 6, 255));wdl->AddLine({ wP.x + sidebarW,wP.y + bodyY }, { wP.x + sidebarW,wP.y + bodyY + bodyH }, IM_COL32(18, 18, 22, 255), 1.0f);
				ImGui::SetCursorPos({ 10,bodyY + 10 });ImGui::BeginChild("##sidebar", { sidebarW - 14.0f,bodyH - 14.0f }, false, 0);
				SearchBox("##search", g_searchBuf, sizeof(g_searchBuf), ImGui::GetContentRegionAvail().x);ImGui::Spacing();GlowSep();
				std::string search = g_searchBuf;std::transform(search.begin(), search.end(), search.begin(), [](unsigned char ch) {return static_cast<char>(std::tolower(ch));});
				for (int i = 0;i < kNavCount;++i) {
					if (!search.empty()) {
						std::string label = kNav[i].label;std::string keys = kNav[i].keywords;
						std::transform(label.begin(), label.end(), label.begin(), [](unsigned char ch) {return static_cast<char>(std::tolower(ch));});
						std::transform(keys.begin(), keys.end(), keys.begin(), [](unsigned char ch) {return static_cast<char>(std::tolower(ch));});
						if (label.find(search) == std::string::npos && keys.find(search) == std::string::npos)continue;
					}
					if (NavItem(kNav[i].label, g_activeSec == i, kNav[i].icon))g_activeSec = i;
				}
				ImGui::EndChild();
			}
			if (!g_compactMode) {
				ImGui::SetCursorPos({ contentX + 10.0f,bodyY + 10.0f });ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.03f,0.03f,0.04f,1 });ImGui::BeginChild("##content_shell", { contentW - 20.0f,bodyH - 14.0f }, false, 0);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_secAlpha);ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (1.0f - g_secAlpha) * 10.0f);
				if (g_activeSec == 0) {
					SectionHeader("Quick Setup", "Fast presets,config save/load,and clean reset controls.");
					const char* presets[] = { "Studio Gloss","Performance Sharp","Cyber Neon","Marble Mirror","Rain Asphalt","Anime Outline","Daylight Clean","Overcast Mood","Volcanic Sunset" };
					ImGui::TextDisabled("Preset");ImGui::SetNextItemWidth(-1);if (ImGui::Combo("##pre", &cfg.fx.presetIdx, presets, IM_ARRAYSIZE(presets)))ApplyPreset(cfg.fx, cfg.fx.presetIdx);
					if (ImGui::Button("Reset All Settings", { -1,30 }))cfg.fx = EffectSettings{};GlowSep();
					ImGui::TextDisabled("Saved Slots");
					for (int sl = 0;sl < 4;++sl) {
						char sb[32], lb[32];sprintf_s(sb, "Save Slot %d", sl + 1);sprintf_s(lb, "Load Slot %d", sl + 1);float hw = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
						if (ImGui::Button(sb, { hw,28 })) { g_userSlots[sl].fx = cfg.fx;g_userSlots[sl].valid = true;SaveConfig(cfg); }ImGui::SameLine(0, 8);if (!g_userSlots[sl].valid)ImGui::BeginDisabled();if (ImGui::Button(lb, { hw,28 }) && g_userSlots[sl].valid)cfg.fx = g_userSlots[sl].fx;if (!g_userSlots[sl].valid)ImGui::EndDisabled();
					}
					GlowSep();if (ImGui::Button("Save Config to File", { -1,30 }))SaveConfigToFile(cfg);if (ImGui::Button("Load Config from File", { -1,30 }))LoadConfigFromFile(cfg);
					if (ImGui::Button("Copy Full JSON", { -1,30 })) { std::string json = EffectSettingsToJson(cfg.fx);if (OpenClipboard(nullptr)) { EmptyClipboard();HGLOBAL hm = GlobalAlloc(GMEM_MOVEABLE, json.size() + 1);if (hm) { memcpy(GlobalLock(hm), json.c_str(), json.size() + 1);GlobalUnlock(hm);SetClipboardData(CF_TEXT, hm); }CloseClipboard(); } }
					static char quick_json_buf[16384] = "";ImGui::InputTextMultiline("##quickjson", quick_json_buf, sizeof(quick_json_buf), { -1,120 });if (ImGui::Button("Paste JSON", { -1,30 })) { if (OpenClipboard(nullptr)) { HANDLE hData = GetClipboardData(CF_TEXT);if (hData) { const char* p = static_cast<const char*>(GlobalLock(hData));if (p) { size_t clen = strlen(p);if (clen > sizeof(quick_json_buf) - 1)clen = sizeof(quick_json_buf) - 1;memcpy(quick_json_buf, p, clen);quick_json_buf[clen] = 0;GlobalUnlock(hData); } }CloseClipboard(); }JsonToEffectSettings(quick_json_buf, cfg.fx); }
				}
				else if (g_activeSec == 1) {
					SectionHeader("Ground Mirror", "Reflective floor controls with sharper UI protection and cleaner fade handling.");
					PremiumToggle("Enable Mirror", &cfg.fx.glossyEnabled, "Turn the reflective floor on or off");PremiumToggle("Protect HUD / Text", &cfg.fx.textProtectEnabled, "Stops UI and bright text from reflecting badly");PremiumToggle("Clear Coat Mode", &cfg.fx.clearCoatMode, "Adds an extra polished top layer");PremiumSlider("gfi", "Mirror Intensity", &cfg.fx.glossyFloorIntensity, 0.f, 3.f, "%.2f", "How strong the reflection is");PremiumSlider("gro", "Roughness", &cfg.fx.glossyRoughness, 0.0001f, 0.05f, "%.4f", "Lower is sharper,higher is blurrier");PremiumSlider("gfr", "Fresnel Power", &cfg.fx.glossyFresnelPower, 1.f, 5.f, "%.1f", "Edge reflectivity strength");PremiumSlider("gsp", "Specular Glint", &cfg.fx.glossySpecularGlint, 0.f, 5.f, "%.1f", "Highlight brightness");PremiumSlider("gco", "Reflection Contrast", &cfg.fx.glossyContrast, 0.8f, 2.0f, "%.2f", "Reflection contrast");PremiumSlider("ghr", "Horizon Height", &cfg.fx.horizonY, 0.02f, 0.85f, "%.2f", "Where reflection starts");PremiumSlider("gfe", "Distance Fade", &cfg.fx.groundFadeEnd, 0.10f, 1.0f, "%.2f", "How far the reflection continues");
				}
				else if (g_activeSec == 2) {
					SectionHeader("Reflection Colors", "Tint reflections and specular highlights.");
					ImGui::TextDisabled("Reflection Tint");ImGui::ColorPicker3("##rt", cfg.fx.glossyTint, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);GlowSep();ImGui::TextDisabled("Specular Tint");ImGui::ColorPicker3("##st", cfg.fx.specularTint, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);GlowSep();
					PremiumSlider("neo", "Neon Amplification", &cfg.fx.neonGlowIntensity, 0.f, 3.f, "%.2f", "Boost reflection neon energy");PremiumSlider("npk", "Pink Boost", &cfg.fx.neonPinkBoost, 0.5f, 2.5f, "%.2f", "Pushes warm neon tones");PremiumSlider("nbf", "Blue Fog Blend", &cfg.fx.neonBlueFog, 0.f, 1.f, "%.2f", "Adds cold blue haze");
				}
				else if (g_activeSec == 3) {
					SectionHeader("Lighting", "Global scene brightness,warmth,horizon glow,and ambient feeling.");
					PremiumSlider("exp", "Exposure", &cfg.fx.exposure, -2.f, 2.f, "%.2f EV", "Brighten or darken the whole scene");PremiumSlider("tmp", "Temperature", &cfg.fx.tempKelvin, 2000.f, 10000.f, "%.0f K", "Warm below 6500K,cool above 6500K");PremiumSlider("vib", "Vibrance", &cfg.fx.vibranceAmount, 0.f, 1.f, "%.2f", "Boost less saturated colors first");PremiumSlider("sky", "Sky Glow", &cfg.fx.skyGlowStrength, 0.f, 2.f, "%.2f", "Adds glow around the horizon");PremiumSlider("sun", "Sun Angle", &cfg.fx.sunAngle, 0.f, 360.f, "%.0f deg", "Directional warm light sweep angle");PremiumSlider("amb", "Ambient Light", &cfg.fx.ambientLight, 0.f, 2.f, "%.2f", "Base ambient strength");
				}
				else if (g_activeSec == 4) {
					SectionHeader("Bloom & Rays", "Glow and volumetric streaks from bright highlights.");
					PremiumSlider("bli", "Bloom Intensity", &cfg.fx.bloomIntensity, 0.f, 2.f, "%.2f", "How strongly highlights bleed");PremiumSlider("blt", "Bloom Threshold", &cfg.fx.bloomThreshold, 0.3f, 1.f, "%.2f", "Only very bright areas will bloom");PremiumSlider("gri", "God Ray Intensity", &cfg.fx.godRayIntensity, 0.f, 2.f, "%.2f", "Volumetric light shaft strength");PremiumSlider("grd", "Ray Falloff", &cfg.fx.godRayDecay, 0.80f, 0.99f, "%.2f", "How fast rays fade");PremiumSlider("grx", "Light Position X", &cfg.fx.godRayX, 0.f, 1.f, "%.2f", "Horizontal light source position");PremiumSlider("gry", "Light Position Y", &cfg.fx.godRayY, 0.f, 1.f, "%.2f", "Vertical light source position");
				}
				else if (g_activeSec == 5) {
					SectionHeader("Anti-Aliasing", "Smoother edges,temporal blending,and crisp sharpening recovery.");
					PremiumToggle("FXAA", &cfg.fx.fxaaEnabled, "Fast single pass edge smoothing");PremiumToggle("Temporal Blend", &cfg.fx.upscaleDLSS, "Frame history smoothing with neighborhood clamp");PremiumToggle("Temporal Sharpen", &cfg.fx.upscaleTemporalSharpen, "Restores detail after temporal blend");PremiumToggle("FSR CAS", &cfg.fx.upscaleFSR, "AMD-style contrast adaptive sharpening");PremiumSlider("fsh", "CAS Sharpness", &cfg.fx.fsrSharpness, 0.f, 1.f, "%.2f", "Strength of CAS sharpening");
				}
				else if (g_activeSec == 6) {
					SectionHeader("Color Grading", "Tone mapping and style themes.");
					const char* maps[] = { "Neutral","ACES Filmic","Reinhard HDR","Uncharted 2" };ImGui::TextDisabled("Tone Mapper");ImGui::SetNextItemWidth(-1);ImGui::Combo("##tm", &cfg.fx.tonemapMode, maps, IM_ARRAYSIZE(maps));
					const char* themes[] = { "Neutral Direct","Ultra Hyper Gloss","Golden Hour Sunset","Cyberpunk Neon Night","Photorealistic Day","Vaporwave Synthwave","Tokyo Drift Night","Matrix Emerald Rain","Noir Cinema","Anime Vivid","Hollywood Teal Orange","Vintage CRT","Pastel Ethereal","Overcast Rain","Infrared Thermal","Sepia Dunes","Arctic Frost Blue","Midnight Obsidian","Royal Emerald","Volcanic Inferno","Amethyst Dusk","HDR Vivid Punch" };ImGui::TextDisabled("Style Theme");ImGui::SetNextItemWidth(-1);ImGui::Combo("##th", &cfg.fx.themeMode, themes, IM_ARRAYSIZE(themes));
				}
				else if (g_activeSec == 7) {
					SectionHeader("Depth Fog", "Distance haze using the available depth texture.");
					PremiumToggle("Enable Fog", &cfg.fx.fogEnabled, "Adds atmospheric distance fog");PremiumSlider("fst", "Fog Start", &cfg.fx.fogDistance, 0.1f, 0.9f, "%.2f", "Depth where fog begins");PremiumSlider("fdn", "Fog Density", &cfg.fx.fogDensity, 0.05f, 1.f, "%.2f", "How fast fog thickens");ImGui::TextDisabled("Fog Color");ImGui::ColorPicker3("##fc", cfg.fx.fogColor, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
				}
				else if (g_activeSec == 8) {
					SectionHeader("Depth of Field", "Focus blur with clean scrolling so the page never looks clipped again.");
					PremiumToggle("Enable DOF", &cfg.fx.dofEnabled, "Blur near and far objects like a camera lens");PremiumSlider("dff", "Focus Distance", &cfg.fx.dofFocusDistance, 0.f, 1.f, "%.2f", "What depth stays sharp");PremiumSlider("dfr", "Focus Range", &cfg.fx.dofRange, 0.01f, 0.5f, "%.2f", "How much stays in focus");PremiumSlider("dfb", "Blur Strength", &cfg.fx.dofBlurStrength, 0.f, 1.f, "%.2f", "Strength of out-of-focus blur");PremiumSlider("tls", "Tilt Focus", &cfg.fx.tiltShift, 0.f, 1.f, "%.2f", "Tilt-shift style focus line");
				}
				else if (g_activeSec == 9) {
					SectionHeader("Sharpening", "Post sharpening after other effects for fine detail recovery.");
					PremiumSlider("sha", "Sharpen Strength", &cfg.fx.sharpeningAmount, 0.f, 2.f, "%.2f", "High values can get crunchy");PremiumSlider("shr", "Sharpen Radius", &cfg.fx.sharpeningRadius, 0.5f, 3.f, "%.2f", "How wide the sharpen kernel reaches");PremiumSlider("mbl", "Motion Blur", &cfg.fx.motionBlurAmount, 0.f, 0.9f, "%.2f", "Frame blend trail strength");
				}
				else if (g_activeSec == 10) {
					SectionHeader("Cel Outlines", "Clean edge outlines with dedicated controls.");
					PremiumToggle("Enable Outlines", &cfg.fx.outlineEnabled, "Draw edge lines from the depth texture");PremiumSlider("olt", "Outline Thickness", &cfg.fx.outlineThickness, 0.5f, 4.f, "%.1f", "How thick the lines are");ImGui::TextDisabled("Outline Color");ImGui::ColorPicker3("##oc", cfg.fx.outlineColor, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
				}
				else if (g_activeSec == 11) {
					SectionHeader("Screen Effects", "Camera and monitor imperfections that actually map to real shader controls.");
					PremiumSlider("ca", "Chromatic Aberration", &cfg.fx.chromaticAberration, 0.f, 3.f, "%.2f", "RGB split toward the edges");PremiumSlider("vin", "Vignette", &cfg.fx.vignetteIntensity, 0.f, 1.f, "%.2f", "Darken corners");PremiumSlider("vis", "Vignette Softness", &cfg.fx.vignetteSmoothness, 0.1f, 1.f, "%.2f", "How soft the vignette fades");PremiumSlider("grn", "Film Grain", &cfg.fx.filmGrainAmount, 0.f, 1.f, "%.2f", "Adds photographic noise");PremiumSlider("scl", "Scanlines", &cfg.fx.scanlineIntensity, 0.f, 1.f, "%.2f", "CRT line shading");PremiumSlider("crt", "CRT Curve", &cfg.fx.crtCurve, 0.f, 0.5f, "%.2f", "Screen curvature");PremiumSlider("gli", "Glitch", &cfg.fx.glitchAmount, 0.f, 1.f, "%.2f", "Horizontal glitch bursts");
				}
				else if (g_activeSec == 12) {
					SectionHeader("Vision Modes", "Special viewing looks driven by the live post-processing pass.");
					PremiumSlider("ngv", "Night Vision", &cfg.fx.nightVision, 0.f, 1.f, "%.2f", "Green boosted night look");PremiumSlider("thr", "Thermal Vision", &cfg.fx.thermalVision, 0.f, 1.f, "%.2f", "Heat-map style grading");
				}
				else if (g_activeSec == 13) {
					SectionHeader("Weather & Waves", "Rain,wind,and water distortion controls. Heavy rain with dense streaks and refraction");
					PremiumSlider("wda", "Wave Distortion", &cfg.fx.waveDistortionAmount, 0.f, 2.f, "%.2f", "Scene ripple amount");PremiumSlider("wsp", "Wave Speed", &cfg.fx.waveSpeed, 0.1f, 4.f, "%.1f", "Animation speed");PremiumSlider("wsc", "Wave Frequency", &cfg.fx.waveScale, 10.f, 100.f, "%.0f", "Number of ripples");
					PremiumSlider("rnd", "Rain Drops", &cfg.fx.rainDrops, 0.f, 1.f, "%.2f", "Vertical drop distortion(now TOP->BOTTOM)");PremiumSlider("wnd", "Wind Distortion", &cfg.fx.windDistortion, 0.f, 1.f, "%.2f", "Sideways gust distortion");
				}
				else if (g_activeSec == 14) {
					SectionHeader("Split Compare", "Side-by-side processed vs original comparison.");
					PremiumToggle("Enable Split Compare", &cfg.fx.splitScreenEnabled, "Left is processed,right is original");PremiumSlider("ssp", "Divider Position", &cfg.fx.splitScreenPos, 0.f, 1.f, "%.2f", "Move the comparison line");
				}
				else if (g_activeSec == 15) {
					SectionHeader("Crosshair", "Fully working center crosshair while the menu is hidden.");
					PremiumToggle("Enable Crosshair", &cfg.fx.crosshairEnabled, "Draws while you play");const char* chStyles[] = { "Cross","Dot","Circle","Cross + Dot" };
					ImGui::TextDisabled("Style");ImGui::SetNextItemWidth(-1);ImGui::Combo("##chs", &cfg.fx.crosshairStyle, chStyles, IM_ARRAYSIZE(chStyles));
					PremiumSlider("chs2", "Size", &cfg.fx.crosshairSize, 2.f, 24.f, "%.0f px", "Length or radius");PremiumSlider("chg", "Gap", &cfg.fx.crosshairGap, 0.f, 16.f, "%.0f px", "Empty center space");PremiumSlider("cht", "Thickness", &cfg.fx.crosshairThickness, 1.f, 6.f, "%.0f px", "Line thickness");PremiumSlider("cho", "Opacity", &cfg.fx.crosshairOpacity, 0.2f, 1.f, "%.2f", "Visibility");
					ImGui::TextDisabled("Crosshair Color");ImGui::ColorPicker3("##chc", cfg.fx.crosshairColor, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
				}
				else if (g_activeSec == 16) {
					SectionHeader("Performance", "Live stats and capture status.");
					ImVec2 pp = ImGui::GetCursorScreenPos();float pw = ImGui::GetContentRegionAvail().x;ImDrawList* pdl = ImGui::GetWindowDrawList();pdl->AddRectFilled(pp, { pp.x + pw,pp.y + 72 }, IM_COL32(8, 8, 12, 255), 8.0f);pdl->AddRect(pp, { pp.x + pw,pp.y + 72 }, IM_COL32(20, 20, 25, 255), 8.0f, 0, 1.0f);
					char fb[32];sprintf_s(fb, "%.0f", psNow.overlayFps);ImU32 fpsC = psNow.overlayFps > 45 ? g_accentCol : (psNow.overlayFps > 25 ? IM_COL32(225, 195, 65, 255) : IM_COL32(215, 85, 85, 255));pdl->AddText({ pp.x + 14,pp.y + 10 }, fpsC, fb);pdl->AddText({ pp.x + 14,pp.y + 33 }, IM_COL32(95, 95, 100, 255), "FPS");
					char gfb[48];sprintf_s(gfb, "Game %.0f fps", g_gameFps.load());pdl->AddText({ pp.x + 14,pp.y + 50 }, IM_COL32(125, 125, 130, 255), gfb);ImGui::SetCursorScreenPos({ pp.x + 96,pp.y + 10 });ImGui::PlotLines("##fpsg", g_fpsHist, 160, g_fpsPos, nullptr, 0.0f, std::max(120.0f, (float)(psNow.overlayFps * 1.2f)), { pw - 110,50 });ImGui::Dummy({ pw,76 });GlowSep();
					ImGui::TextDisabled("Detected GPUs");if (gpus.empty())ImGui::BulletText("No dedicated GPU found");else for (auto& g : gpus) { std::string gn = WtoA(g.name);double vg = static_cast<double>(g.vramBytes) / (1024.0 * 1024.0 * 1024.0);ImGui::BulletText("%s | %.1f GB VRAM", gn.c_str(), vg); }
					GlowSep();ImGui::BulletText(captureOk ? "DXGI screen capture active" : "Screen capture unavailable | effects will not show without capture");PremiumToggle("Auto Track Roblox Window", &cfg.autoTrackWindow, "Overlay follows the Roblox window automatically");
				}
				else if (g_activeSec == 17) {
					SectionHeader("Engine", "Startup,error handling,and renderer status.");
					cfg.fx.engineMode = 0;
					ImGui::TextColored(ImVec4(1, 0.75f, 0.2f, 1), "AI Depth Safe Mode is not supported in this build.");
					ImGui::TextDisabled("Renderer: Pure GPU shaders | no AI depth path.");
					ImGui::Spacing();
					ImGui::TextDisabled("%s", g_aiStatus.c_str());
					GlowSep();
					ImGui::TextDisabled("Game FPS");ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(g_accentCol), "%.0f fps", g_gameFps.load());
				}
				else if (g_activeSec == 18) {
					SectionHeader("AI Upscale", "Five real downloadable ONNX upscale models with direct load status.");
					PremiumToggle("Enable AI Upscale", &cfg.fx.upscaleNAFNet, "Uses the selected super-resolution model when available");
					if (cfg.fx.upscaleNAFNet) {
						for (int mi = 0;mi < kNumAiModels;++mi) { if (ImGui::RadioButton(kAiModels[mi].displayName, g_selModel == mi)) { g_selModel = mi;aiInit = false;aiProv[0].Reset(); } }
						GlowSep();PremiumSlider("bs", "Blend Strength", &cfg.fx.upscaleStrength, 0.05f, 1.f, "%.2f", "How strongly AI output blends in");const char* qLvls[] = { "Performance","Balanced","Quality","Max Quality" };ImGui::TextDisabled("Quality Target");ImGui::SetNextItemWidth(-1);ImGui::Combo("##ql", &cfg.fx.upscaleQuality, qLvls, IM_ARRAYSIZE(qLvls));
#if SR_HAS_ONNX
						bool busy = aiProv[0].State() == ProvState::Downloading || aiProv[0].State() == ProvState::Verifying;bool failed = aiProv[0].State() == ProvState::Failed;
						if (!aiInit && !busy) { if (ImGui::Button("Load Selected Model", { -1,30 })) { const auto& def = kAiModels[g_selModel];aiProv[0].Begin({ def.url1,def.url2 }, def.localPath, GetAiModelMinBytes(def)); } }
						if (busy) { char pb[96];sprintf_s(pb, "Downloading %d%%", static_cast<int>(aiProv[0].Prog() * 100.0));ImGui::ProgressBar(static_cast<float>(aiProv[0].Prog()), { -1,16 }, pb); }
						if (failed) { ImGui::TextColored(ImVec4(1, .45f, .45f, 1), "Download failed. Check connection.");if (ImGui::Button("Retry Download", { -1,28 }))aiProv[0].Retry(); }
						if (aiInit)ImGui::TextColored(ImVec4(.5f, .9f, .55f, 1), "AI Upscale active | %s", aiEngine.UsingCpu() ? "CPU mode" : "GPU(DirectML)");
						else if (aiProv[0].State() == ProvState::Ready)ImGui::TextColored(ImVec4(1, .8f, .2f, 1), "Model ready. Initializing...");else if (!aiEngine.LastErr().empty())ImGui::TextColored(ImVec4(1, .45f, .45f, 1), "AI Error: %s", aiEngine.LastErr().c_str());
#else
						ImGui::TextColored(ImVec4(1, 0.75f, 0.2f, 1), "AI upscale is disabled in this build because ONNX Runtime / DirectML headers are not available.");
#endif
					}
				}
				else if (g_activeSec == 19) {
					SectionHeader("Support", "Community links and controls.");if (!g_discordSrv)LoadDiscordTexture(g_dev);ImDrawList* sdl = ImGui::GetWindowDrawList();ImVec2 sp = ImGui::GetCursorScreenPos();float sw2 = ImGui::GetContentRegionAvail().x;sdl->AddRectFilled(sp, { sp.x + sw2,sp.y + 86 }, IM_COL32(10, 10, 28, 255), 8.0f);sdl->AddRect(sp, { sp.x + sw2,sp.y + 86 }, IM_COL32(50, 60, 120, 180), 8.0f, 0, 1.0f);
					ImVec2 ip = { sp.x + 16,sp.y + 16 };float isz = 48;sdl->AddRectFilled(ip, { ip.x + isz,ip.y + isz }, IM_COL32(88, 101, 242, 255), 10.0f);if (g_discordSrv && g_discordW > 0)sdl->AddImage((ImTextureID)g_discordSrv, { ip.x,ip.y }, { ip.x + isz,ip.y + isz }, { 0,0 }, { 1,1 }, IM_COL32(255, 255, 255, 255));else DrawDiscordLogo(sdl, { ip.x + isz * 0.5f,ip.y + isz * 0.5f }, isz * 0.6f, IM_COL32(255, 255, 255, 255));
					sdl->AddText({ sp.x + 76,sp.y + 16 }, IM_COL32(205, 205, 255, 255), "Join the StudWorks Discord");sdl->AddText({ sp.x + 76,sp.y + 38 }, IM_COL32(140, 140, 200, 255), "Get updates,presets,help,and community support.");ImGui::Dummy({ sw2,86 });
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.345f, 0.396f, 0.949f, 1));ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.47f, 1, 1));ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.33f, 0.80f, 1));bool btnClicked = ImGui::Button(g_linkCopied ? "Link copied" : "Copy Discord Link", { -1,38 });ImGui::PopStyleColor(3);
					if (btnClicked) { const char* lnk = "https://discord.gg/studworks";if (OpenClipboard(nullptr)) { EmptyClipboard();HGLOBAL hm = GlobalAlloc(GMEM_MOVEABLE, strlen(lnk) + 1);if (hm) { memcpy(GlobalLock(hm), lnk, strlen(lnk) + 1);GlobalUnlock(hm);SetClipboardData(CF_TEXT, hm); }CloseClipboard();g_linkCopied = true;g_linkCopiedTimer = 2.0f; } }
					GlowSep();ImGui::TextDisabled("Version 1.0 Beta");ImGui::BulletText("INSERT | toggle menu");ImGui::BulletText("END | quit");
				}
				else if (g_activeSec == 20) {
					SectionHeader("Prism Split", "Directional RGB splitting for glass-like chroma separation.");
					PremiumSlider("cf0", "Split Strength", &cfg.fx.customFx[0], 0.f, 1.f, "%.2f", "How far red and blue channels separate");PremiumSlider("cf1", "Split Angle", &cfg.fx.customFx[1], 0.f, 1.f, "%.2f", "Rotation of the prism direction");
				}
				else if (g_activeSec == 21) {
					SectionHeader("Halftone Studio", "Print-style dot shading for stylized comic or poster looks.");
					PremiumSlider("cf2", "Dot Scale", &cfg.fx.customFx[2], 0.f, 1.f, "%.2f", "Size of the halftone cells");PremiumSlider("cf3", "Dot Mix", &cfg.fx.customFx[3], 0.f, 1.f, "%.2f", "Blend amount into the scene");
				}
				else if (g_activeSec == 22) {
					SectionHeader("Pixel Lab", "Blocky retro pixel look independent from every other tab.");
					PremiumSlider("cf4", "Pixel Size", &cfg.fx.customFx[4], 0.f, 1.f, "%.2f", "How large the pixel blocks are");PremiumSlider("cf5", "Pixel Mix", &cfg.fx.customFx[5], 0.f, 1.f, "%.2f", "How strongly the pixelated image replaces the original");
				}
				else if (g_activeSec == 23) {
					SectionHeader("Poster Lab", "Hard color step reduction for a posterized render.");
					PremiumSlider("cf6", "Level Count", &cfg.fx.customFx[6], 0.f, 1.f, "%.2f", "Higher means more color steps");PremiumSlider("cf7", "Poster Mix", &cfg.fx.customFx[7], 0.f, 1.f, "%.2f", "Blend strength of the poster effect");
				}
				else if (g_activeSec == 24) {
					SectionHeader("Embossed Film", "Directional raised-edge emboss without reusing outline mode.");
					PremiumSlider("cf8", "Emboss Strength", &cfg.fx.customFx[8], 0.f, 1.f, "%.2f", "Raised relief intensity");PremiumSlider("cf9", "Emboss Mix", &cfg.fx.customFx[9], 0.f, 1.f, "%.2f", "Blend strength");
				}
				else if (g_activeSec == 25) {
					SectionHeader("Duotone Wash", "Maps the whole scene between two cinematic color anchors.");
					PremiumSlider("cf10", "Duotone Mix", &cfg.fx.customFx[10], 0.f, 1.f, "%.2f", "How much the duotone overrides the scene");PremiumSlider("cf11", "Warm-Cool Bias", &cfg.fx.customFx[11], 0.f, 1.f, "%.2f", "Shifts between warm amber and cool blue");
				}
				else if (g_activeSec == 26) {
					SectionHeader("Light Streaks", "Bright highlights stretch into directional streaks.");
					PremiumSlider("cf12", "Streak Strength", &cfg.fx.customFx[12], 0.f, 1.f, "%.2f", "How bright and visible the streaks are");PremiumSlider("cf13", "Streak Direction", &cfg.fx.customFx[13], 0.f, 1.f, "%.2f", "Angle of the light sweep");
				}
				else if (g_activeSec == 27) {
					SectionHeader("VHS Deck", "Tape-style wobble and chroma crawl.");
					PremiumSlider("cf14", "Tape Jitter", &cfg.fx.customFx[14], 0.f, 1.f, "%.2f", "Horizontal tape wobble");PremiumSlider("cf15", "Color Crawl", &cfg.fx.customFx[15], 0.f, 1.f, "%.2f", "Color bleed and noise amount");
				}
				else if (g_activeSec == 28) {
					SectionHeader("Scan Pulse", "Animated scanning band that sweeps the screen.");
					PremiumSlider("cf16", "Pulse Width", &cfg.fx.customFx[16], 0.f, 1.f, "%.2f", "How wide the scanning band is");PremiumSlider("cf17", "Pulse Intensity", &cfg.fx.customFx[17], 0.f, 1.f, "%.2f", "Brightness of the pulse");
				}
				else if (g_activeSec == 29) {
					SectionHeader("Heat Haze", "Localized shimmering air distortion.");
					PremiumSlider("cf18", "Haze Amount", &cfg.fx.customFx[18], 0.f, 1.f, "%.2f", "Distortion strength");PremiumSlider("cf19", "Haze Speed", &cfg.fx.customFx[19], 0.f, 1.f, "%.2f", "How quickly the haze moves");
				}
				else if (g_activeSec == 30) {
					SectionHeader("Edge Glow", "Luminous screen-space glow around contrast edges.");
					PremiumSlider("cf20", "Glow Strength", &cfg.fx.customFx[20], 0.f, 1.f, "%.2f", "Edge brightness");PremiumSlider("cf21", "Glow Width", &cfg.fx.customFx[21], 0.f, 1.f, "%.2f", "How wide the glow reaches");
				}
				else if (g_activeSec == 31) {
					SectionHeader("Color Isolate", "Keeps one hue alive while muting everything else.");
					PremiumSlider("cf22", "Target Hue", &cfg.fx.customFx[22], 0.f, 1.f, "%.2f", "Select which hue remains colorful");PremiumSlider("cf23", "Hue Range", &cfg.fx.customFx[23], 0.f, 1.f, "%.2f", "How much surrounding hue is preserved");
				}
				else if (g_activeSec == 32) {
					SectionHeader("Luma Fade", "Brightness-based tinting that only affects selected luma bands.");
					PremiumSlider("cf24", "Fade Threshold", &cfg.fx.customFx[24], 0.f, 1.f, "%.2f", "Where the luma fade starts");PremiumSlider("cf25", "Tint Amount", &cfg.fx.customFx[25], 0.f, 1.f, "%.2f", "How strong the tint overlay becomes");
				}
				else if (g_activeSec == 33) {
					SectionHeader("Crystal Mosaic", "Hex-like glass block breakup for stylized refraction.");
					PremiumSlider("cf26", "Cell Size", &cfg.fx.customFx[26], 0.f, 1.f, "%.2f", "How large the crystal cells are");PremiumSlider("cf27", "Crystal Mix", &cfg.fx.customFx[27], 0.f, 1.f, "%.2f", "Blend amount");
				}
				else if (g_activeSec == 34) {
					SectionHeader("Radial Tint", "Applies a center-out tint gradient independent of vignette.");
					PremiumSlider("cf28", "Tint Strength", &cfg.fx.customFx[28], 0.f, 1.f, "%.2f", "How strongly the gradient colors the image");PremiumSlider("cf29", "Tint Radius", &cfg.fx.customFx[29], 0.f, 1.f, "%.2f", "How quickly the tint spreads outward");
				}
				else if (g_activeSec == 35) {
					SectionHeader("Shadow Crush", "Deepens lower luma ranges without touching highlights the same way.");
					PremiumSlider("cf30", "Crush Amount", &cfg.fx.customFx[30], 0.f, 1.f, "%.2f", "Darken shadows with more punch");PremiumSlider("cf31", "Crush Pivot", &cfg.fx.customFx[31], 0.f, 1.f, "%.2f", "Sets the luma pivot for crushing");
				}
				else if (g_activeSec == 36) {
					SectionHeader("Highlight Softener", "Bloom-like highlight diffusing without reusing the bloom tab.");
					PremiumSlider("cf32", "Soften Amount", &cfg.fx.customFx[32], 0.f, 1.f, "%.2f", "How much bright regions spread");PremiumSlider("cf33", "Highlight Threshold", &cfg.fx.customFx[33], 0.f, 1.f, "%.2f", "Brightness required before softening starts");
				}
				else if (g_activeSec == 37) {
					SectionHeader("Tunnel Vision", "Center emphasis effect with controllable peripheral falloff.");
					PremiumSlider("cf34", "Tunnel Radius", &cfg.fx.customFx[34], 0.f, 1.f, "%.2f", "Size of the clear center area");PremiumSlider("cf35", "Tunnel Feather", &cfg.fx.customFx[35], 0.f, 1.f, "%.2f", "How soft the edge falloff is");
				}
				else if (g_activeSec == 38) {
					SectionHeader("Corner Warp", "Pulls and bends only the screen corners for stylized framing.");
					PremiumSlider("cf36", "Warp Amount", &cfg.fx.customFx[36], 0.f, 1.f, "%.2f", "Corner pull strength");PremiumSlider("cf37", "Warp Curve", &cfg.fx.customFx[37], 0.f, 1.f, "%.2f", "How curved the warp becomes");
				}
				ImGui::PopStyleVar();ImGui::EndChild();ImGui::PopStyleColor();
			}
			float footY = wS.y - footerH;wdl->AddLine({ wP.x,wP.y + footY }, { wP.x + wS.x,wP.y + footY }, IM_COL32(16, 16, 20, 255), 1.0f);
			ImU32 fpsC2 = psNow.overlayFps > 45 ? g_accentCol : (psNow.overlayFps > 25 ? IM_COL32(220, 190, 60, 255) : IM_COL32(210, 80, 80, 255));wdl->AddCircleFilled({ wP.x + 16,wP.y + footY + footerH * 0.5f }, 3.5f, fpsC2);
			char fbuf[160];
			if (g_liveEnabled)sprintf_s(fbuf, "Game %.0f fps | AI %.1f fps | %s | StudReshader 1.0 Beta", g_gameFps.load(), g_liveFps.load(), cfg.fx.engineMode == 1 ? "AI mode" : "GPU mode");
			else sprintf_s(fbuf, "Game %.0f fps | %s | StudReshader 1.0 Beta", g_gameFps.load(), cfg.fx.engineMode == 1 ? "AI mode" : "GPU mode");
			wdl->AddText({ wP.x + 26,wP.y + footY + (footerH - ImGui::GetTextLineHeight()) * 0.5f }, IM_COL32(150, 150, 155, 255), fbuf);
			ImGui::SetCursorPos({ wS.x - 108,footY + 8 });ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6);if (ImGui::Button("Shutdown", { 92,26 }))g_appRunning = false;ImGui::PopStyleVar();
			g_panelRect = { static_cast<LONG>(rbxRect.left + wP.x),static_cast<LONG>(rbxRect.top + wP.y),static_cast<LONG>(rbxRect.left + wP.x + wS.x),static_cast<LONG>(rbxRect.top + wP.y + wS.y) };
			ImGui::End();ImGui::PopStyleColor();ImGui::PopStyleVar(2);
		}
		else g_panelRect = { 0,0,0,0 };
		if (g_radarOn) {
			RECT rr{};
			if (GetWindowRect(g_robloxHwnd, &rr)) {
				int rw = rr.right - rr.left, rh = rr.bottom - rr.top;
				std::vector<VisionDetection> dets;
				int dfw = 0, dfh = 0;
				{
					std::lock_guard<std::mutex> lk(g_radarMtx);
					dets = g_radarDets;dfw = g_radarFW;dfh = g_radarFH;
				}
				if (dfw > 0 && dfh > 0 && rw > 0 && rh > 0) {
					float sx = (float)rw / (float)dfw, sy = (float)rh / (float)dfh;
					ImDrawList* dl = ImGui::GetForegroundDrawList();
					for (auto& d : dets) {
						float bx0 = std::max(0.0f, d.cx - d.w * 0.5f), by0 = std::max(0.0f, d.cy - d.h * 0.5f);
						float bx1 = std::min((float)dfw, d.cx + d.w * 0.5f), by1 = std::min((float)dfh, d.cy + d.h * 0.5f);
						ImVec2 p0((float)rr.left + bx0 * sx, (float)rr.top + by0 * sy);
						ImVec2 p1((float)rr.left + bx1 * sx, (float)rr.top + by1 * sy);
						ImU32 col = IM_COL32(235, 60, 60, 255);
						dl->AddRect(p0, p1, col, 2.0f, 0, 2.5f);
						if (d.conf > 0.25f) {
							char lbl[32];
							sprintf_s(lbl, "P%.0f%%", d.conf * 100.0f);
							ImVec2 ts = ImGui::CalcTextSize(lbl);
							dl->AddText({ p0.x,p0.y - ts.y - 2.0f }, IM_COL32(235, 90, 90, 255), lbl);
						}
					}
				}
			}
		}
		if (g_coachOn) {
			RECT rr{};
			if (GetWindowRect(g_robloxHwnd, &rr)) {
				int rw = rr.right - rr.left, rh = rr.bottom - rr.top;
				std::vector<VisionDetection> dets;
				int dfw = 0, dfh = 0;
				{
					std::lock_guard<std::mutex> lk(g_radarMtx);
					dets = g_radarDets;dfw = g_radarFW;dfh = g_radarFH;
				}
				std::string coachTxt;
				{
					std::lock_guard<std::mutex> lk(g_coachTextMtx);
					coachTxt = g_coachAiText;
				}
				if (!g_coachAiBusy.load() && !g_brainRun.load()) {
					double nowT = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
					bool hasTargets = !dets.empty() && dfw > 0 && dfh > 0;
					bool needAsk = false;
					if (coachTxt.empty())
						needAsk = (nowT - g_coachLastAskT > 3.0);
					else
						needAsk = hasTargets && (nowT - g_coachLastAskT > 4.0);
					if (needAsk) {
						g_coachLastAskT = nowT;
						VisionDetection* np2 = nullptr;
						if (hasTargets) {
							float bd2 = 1e9f;
							for (auto& d : dets) {
								float ddx = d.cx - dfw * 0.5f, ddy = d.cy - dfh * 0.5f;
								float dd = ddx * ddx + ddy * ddy;
								if (dd < bd2) { bd2 = dd;np2 = &d; }
							}
						}
						if (np2) {
							int cid2 = (int)(np2->cx * 97.0f) ^ (int)(np2->cy * 31.0f);
							g_coachTargetId = cid2;
							CoachAiAsk(cid2, np2->cx - dfw * 0.5f, np2->cy - dfh * 0.5f, dfw, dfh);
						}
						else if (coachTxt.empty()) {
							g_coachTargetId = -1;
							CoachAiAsk(-1, 0.0f, 0.0f, dfw > 0 ? dfw : 1, dfh > 0 ? dfh : 1);
						}
					}
				}
				if (!coachTxt.empty()) {
					DetectKeyboardLayout();
					float winW = 330.0f;
					float fixX = (float)rr.left + (rw - winW) * 0.5f;
					float fixY = (float)rr.top + 70.0f;
					ImU32 bord = IM_COL32((int)(g_coachCol[0] * 255), (int)(g_coachCol[1] * 255), (int)(g_coachCol[2] * 255), 200);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
					ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(10.0f / 255.0f, 12.0f / 255.0f, 18.0f / 255.0f, g_coachOpacity));
					ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(bord));
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
					ImGui::SetNextWindowPos({ fixX,fixY }, ImGuiCond_Always);
					ImGui::SetNextWindowSize({ winW,0 }, ImGuiCond_Always);
					ImGui::Begin("##coach", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
					ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.6f, 1.0f), "COACH");
					ImGui::SetWindowFontScale(std::min(g_coachFont / 16.0f, 1.15f));
					char keyCh = 0;
					size_t kp = coachTxt.find('(');
					if (kp != std::string::npos && kp + 2 < coachTxt.size() && coachTxt[kp + 2] == ')')keyCh = coachTxt[kp + 1];
					ImGui::TextWrapped("%s", coachTxt.c_str());
					ImGui::SetWindowFontScale(1.0f);
					if (keyCh) {
						ImGui::Spacing();
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.22f, 0.3f, 1));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.26f, 0.36f, 1));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
						char kb[4] = { keyCh,0 };
						ImGui::Button(kb, { 56,44 });
						ImGui::PopStyleColor(3);
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(0.6f, 0.62f, 0.7f, 1), "press this key");
					}
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.62f, 0.15f, 0.15f, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.2f, 0.2f, 1));
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
					if (ImGui::Button("STOP COACH", { -1,30 })) {
						g_coachOn = false;
						{ std::lock_guard<std::mutex> ck(g_coachTextMtx);g_coachAiText.clear(); }
					}
					ImGui::PopStyleColor(3);
					ImGui::End();
					ImGui::PopStyleColor(3);
					ImGui::PopStyleVar(2);
				}
				else {
					float winW = 300.0f;
					float fixX = (float)rr.left + (rw - winW) * 0.5f;
					float fixY = (float)rr.top + 70.0f;
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
					ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(10.0f / 255.0f, 12.0f / 255.0f, 18.0f / 255.0f, g_coachOpacity));
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.92f, 1.0f));
					ImGui::SetNextWindowPos({ fixX,fixY }, ImGuiCond_Always);
					ImGui::SetNextWindowSize({ winW,0 }, ImGuiCond_Always);
					ImGui::Begin("##coach", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
					ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.6f, 1.0f), "COACH");
					if (g_coachAiBusy.load())
						ImGui::TextWrapped("Coach on | thinking...");
					else
						ImGui::TextWrapped("Coach on | ask the AI to guide you");
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.62f, 0.15f, 0.15f, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.2f, 0.2f, 1));
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
					if (ImGui::Button("STOP COACH", { -1,30 })) {
						g_coachOn = false;
						{ std::lock_guard<std::mutex> ck(g_coachTextMtx);g_coachAiText.clear(); }
					}
					ImGui::PopStyleColor(3);
					ImGui::End();
					ImGui::PopStyleColor(2);
					ImGui::PopStyleVar();
				}
			}
		}
		if (g_liveEnabled && g_liveModel == 4 && g_objReady && g_objSrv) {
			RECT rr{};
			if (GetWindowRect(g_robloxHwnd, &rr)) {
				float sx = (rr.right - rr.left) > 0 ? (float)(rr.right - rr.left) / (float)kLiveDsW : 1.0f;
				float sy = (rr.bottom - rr.top) > 0 ? (float)(rr.bottom - rr.top) / (float)kLiveDsH : 1.0f;
				float cx = 0.0f, cy = 0.0f, cw = 0.0f, ch = 0.0f, sc = 1.0f;
				{
					std::lock_guard<std::mutex> lk(g_objMtx);
					cx = g_objCX;cy = g_objCY;cw = g_objCW;ch = g_objCH;sc = g_objScale;
				}
				float w = cw * sx * sc * 0.6f, h = ch * sy * sc * 0.6f;
				ImVec2 p0((float)rr.left + cx * sx, (float)rr.top + cy * sy);
				ImGui::GetForegroundDrawList()->AddImage((ImTextureID)g_objSrv, p0, { p0.x + w,p0.y + h });
			}
		}
		ImGui::Render();
		if (g_rtv && g_ctx && needWork) {
			std::lock_guard<std::mutex> ctxLk(g_ctxMtx);const float clear[4] = { 0,0,0,0 };g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);g_ctx->ClearRenderTargetView(g_rtv, clear);
			bool sceneFx = cfg.fx.glossyEnabled || cfg.fx.upscaleSD || cfg.fx.upscaleNAFNet || cfg.fx.splitScreenEnabled ||
				cfg.fx.outlineEnabled || cfg.fx.fogEnabled || cfg.fx.dofEnabled || cfg.fx.ssaoEnabled ||
				cfg.fx.upscaleFSR || cfg.fx.upscaleDLSS || cfg.fx.fxaaEnabled ||
				cfg.fx.bloomIntensity > 0.001f || cfg.fx.vignetteIntensity > 0.001f || cfg.fx.filmGrainAmount > 0.001f ||
				cfg.fx.chromaticAberration > 0.001f || cfg.fx.neonGlowIntensity > 0.001f || cfg.fx.skyGlowStrength > 0.001f ||
				cfg.fx.rainDrops > 0.001f || cfg.fx.snowAmount > 0.001f || cfg.fx.thermalVision > 0.001f ||
				cfg.fx.nightVision > 0.001f || cfg.fx.scanlineIntensity > 0.001f || cfg.fx.crtCurve > 0.001f ||
				cfg.fx.glitchAmount > 0.001f || cfg.fx.waveDistortionAmount > 0.001f || cfg.fx.motionBlurAmount > 0.01f ||
				cfg.fx.godRayIntensity > 0.005f || cfg.fx.contactShadow > 0.001f || g_gpuDrawMode || g_visionEnabled || g_liveEnabled || g_radarOn;
			if (sceneFx) {
				D3D11_RECT sc{};sc.left = 0;sc.top = 0;sc.right = static_cast<LONG>(g_W);sc.bottom = static_cast<LONG>(g_H);float curT = std::chrono::duration<float>(std::chrono::steady_clock::now() - g_startTime).count();
				comp.Render(g_ctx, g_rtv, cfg.fx, (g_visionEnabled || g_gpuDrawMode) ? &g_activeStyle : nullptr, g_W, g_H, sc, curT, aiInit && aiEngine.HasOutput());
				if (cfg.fx.upscaleDLSS || cfg.fx.motionBlurAmount > 0.01f)comp.CaptureHistory(g_dev, g_ctx, g_rtv, g_W, g_H);
			}
			if (g_showMenu || g_showSplash || cfg.fx.crosshairEnabled || g_showCloseToast || (g_liveEnabled && g_liveModel == 4) || g_radarOn) { g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());ID3D11RenderTargetView* nullRtv[1] = { nullptr };g_ctx->OMSetRenderTargets(1, nullRtv, nullptr); }
			if (g_sc) { HRESULT pr = g_sc->Present(0, DXGI_PRESENT_DO_NOT_WAIT);if (pr == DXGI_ERROR_WAS_STILL_DRAWING) {} }
			if (g_dcd)g_dcd->Commit();
			g_gameFrames++;
			double nw2 = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
			if (nw2 - g_gameT0.load() >= 1.0) {
				double dt = nw2 - g_gameT0.load();
				g_gameFps.store(dt > 0.0 ? (double)g_gameFrames.load() / dt : 0.0);
				g_gameT0.store(nw2);
				g_gameFrames.store(0);
			}
		}
		if (g_liveEnabled || g_visionEnabled || g_radarOn)LivePublishFrame(comp);
		if (g_radarOn)RadarPublishFrame(comp);
		{
			if (g_visionEnabled && g_visionRun.load()) {
				double nowS = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
				if (nowS - g_lastVisionBeat.load() > 30.0) {
					g_visionRun.store(false);
					if (g_visionThread.joinable())g_visionThread.detach();
					g_visionEnabled = false;
					ResetVisionFx(cfg);
					g_sdStatus = "AI engine stopped | timeout";
				}
			}
			if (g_brainThinking.load()) {
				double nowS = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
				if (nowS - g_brainThinkStart > 180.0) {
					g_brainThinking.store(false);
					g_brainRun.store(false);
					g_brainCancel.store(true);
					if (g_brainThread.joinable() && g_brainThreadDone.load())g_brainThread.join();
					SetDraft("");
					SetStatus("Assistant timed out | CPU is slow | fix CUDA versions for speed");
					BrainAddMsg("assistant", "(timed out)");
				}
			}
			if (g_liveEnabled && g_liveRun.load()) {
				double nowS = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_startTime).count();
				double hwLimit = 30.0;
				if (g_liveModel == 2 && !g_mbEngine.OnGpu())hwLimit = 180.0;
				else if (!g_sdPipeline.FastOnGpu())hwLimit = 60.0;
				double lastBeat = g_lastWorkerBeat.load();
				if (nowS - lastBeat > hwLimit) {
					g_liveRun.store(false);
					g_sdCancel.store(true);
					WaitLiveThread(10);
					g_liveEnabled = false;
					g_sdStatus = "AI engine stopped | timeout";
				}
			}
		}
		{
			double loopMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - loopT0).count();
			if (needWork) { Sleep(0); }
			else { double target = 4.0;if (loopMs < target)Sleep((DWORD)(target - loopMs)); }
		}
	}
	if (g_brainRun.load() || g_brainThinking.load())BrainStop();
	if (g_coachThread.joinable()) {
		for (int i = 0;i < 400 && !g_coachThreadDone.load();++i)Sleep(5);
		if (g_coachThreadDone.load())g_coachThread.join();else g_coachThread.detach();
	}
	if (g_radarRun.load()) {
		g_radarRun.store(false);
		for (int i = 0;i < 400 && !g_radarThreadDone.load();++i)Sleep(5);
		if (g_radarThread.joinable() && g_radarThreadDone.load())g_radarThread.join();
	}
	if (g_visionRun.load()) { g_visionRun.store(false);if (g_visionThread.joinable())g_visionThread.detach(); }
	if (g_liveRun.load()) {
		g_liveRun.store(false);
		g_sdCancel.store(true);
		WaitLiveThread(60);
		if (g_liveThread.joinable() && g_liveThreadDone.load())g_liveThread.join();
	}
	g_liveStaging.Reset();
	cap.Stop();
	g_sdCancel.store(true);
	if (g_sdThread.joinable())g_sdThread.join();
	ImGui_ImplDX11_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();
	if (g_rtv) { g_rtv->Release();g_rtv = nullptr; }
	if (g_dcd) { g_dcd->Release();g_dcd = nullptr; }
	if (g_sc) { g_sc->Release();g_sc = nullptr; }
	if (g_ctx) { g_ctx->Release();g_ctx = nullptr; }
	if (g_dev) { g_dev->Release();g_dev = nullptr; }
	DestroyWindow(g_overlayHwnd);
	timeEndPeriod(1);
	if (g_singleInstanceMutex)CloseHandle(g_singleInstanceMutex);
	return 0;
}
