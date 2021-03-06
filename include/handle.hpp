#pragma once
#include "declare.hpp"
#include "EasyBMP.h"

namespace NHandle
{
struct STrie
{
    int val;
    SPosition pos;
    std::vector<std::pair<int, STrie *>> ve;

    STrie(int _val, SPosition _pos) : val(_val), pos(_pos) {}
} * board, *mine, *face, *boardxp, *minexp, *facexp, *board98, *mine98, *face98;

struct SPixel
{
    int r, g, b;

    SPixel(int _r, int _g, int _b) : r(_r), g(_g), b(_b) {}

    int hash()
    {
        return (r << 16) | (g << 8) | b;
    }
};

SPixel conv(RGBApixel *px)
{
    return SPixel((int)px->Red, (int)px->Green, (int)px->Blue);
}

SPixel scan(int x, int y)
{
    HDC hdc = GetDC(NULL);
    COLORREF px = GetPixel(hdc, x, y);
    ReleaseDC(NULL, hdc);
    return SPixel(GetRValue(px), GetGValue(px), GetBValue(px));
}

// Get the value of the current element given a determine tree
int read(const SPosition &u, STrie *cur)
{
    if (cur->val != -1)
        return cur->val;
    int hsh = scan(u.x + cur->pos.x, u.y + cur->pos.y).hash();
    for (std::pair<int, STrie *> &v : cur->ve)
        if (v.first == hsh)
            return read(u, v.second);
    return -1;
}

void click(const SPosition &u)
{
    SetCursorPos(u.x, u.y);
    mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    st = read(SPosition(win.left + 66, win.top + 61), face);
    // Sleep(5);
}

// Construct a determine tree for a set of elements (board number, mine number, status face)
STrie *construct(std::vector<int> ve, BMP &bmp, SPosition st, int h, int w, int dis)
{
    if (ve.size() == 1)
        return new STrie(ve.back(), SPosition());
    std::pair<unsigned int, SPosition> ans = std::make_pair(0, SPosition());
    std::vector<int> hsh;
    for (int x = 0; x < h; x++)
        for (int y = 0; y < w; y++)
        {
            hsh.clear();
            for (int &v : ve)
                hsh.push_back(conv(bmp(st.x + dis * v + x, st.y + y)).hash());
            sort(hsh.begin(), hsh.end());
            hsh.resize(std::distance(hsh.begin(), std::unique(hsh.begin(), hsh.end())));
            ans = std::max(ans, std::make_pair((unsigned int)hsh.size(), SPosition(x, y)));
        }
    if (ans.first == 1)
        return nullptr;
    STrie *board = new STrie(-1, ans.second);
    hsh.clear();
    for (int &v : ve)
        hsh.push_back(conv(bmp(st.x + dis * v + board->pos.x, st.y + board->pos.y)).hash());
    sort(hsh.begin(), hsh.end());
    hsh.resize(std::distance(hsh.begin(), std::unique(hsh.begin(), hsh.end())));
    for (int &u : hsh)
    {
        std::vector<int> ne;
        for (int &v : ve)
            if (conv(bmp(st.x + dis * v + board->pos.x, st.y + board->pos.y)).hash() == u)
                ne.push_back(v);
        board->ve.push_back(std::make_pair(u, construct(ne, bmp, st, h, w, dis)));
        if (board->ve.back().second == nullptr)
            return nullptr;
    }
    return board;
}

// Initialize default/custom skin
bool init_skin()
{
    BMP bmp;
    bmp.ReadFromFile("data/winxp.bmp");
    boardxp = construct({0, 1, 2, 3, 4, 5, 6, 7, 8}, bmp, SPosition(0, 0), 16, 16, 16);
    minexp = construct({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, bmp, SPosition(0, 33), 11, 21, 12);
    facexp = construct({0, 2, 3}, bmp, SPosition(0, 55), 26, 26, 27);
    bmp.ReadFromFile("data/win98.bmp");
    board98 = construct({0, 1, 2, 3, 4, 5, 6, 7, 8}, bmp, SPosition(0, 0), 16, 16, 16);
    mine98 = construct({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, bmp, SPosition(0, 33), 11, 21, 12);
    face98 = construct({0, 2, 3}, bmp, SPosition(0, 55), 26, 26, 27);
    HKEY reg;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Brightsoft\\Minesweeper X"), 0, KEY_QUERY_VALUE, &reg) != ERROR_SUCCESS)
    {
        board = boardxp;
        mine = minexp;
        face = facexp;
        RegCloseKey(reg);
        return true;
    }
    char val[SZ];
    DWORD len = SZ;
    RegQueryValueEx(reg, TEXT("Skin"), NULL, NULL, (LPBYTE)&val, &len);
    if (val[0] == 0)
    {
        board = boardxp;
        mine = minexp;
        face = facexp;
        RegCloseKey(reg);
        return true;
    }
    if (val[0] == 1)
    {
        board = board98;
        mine = mine98;
        face = face98;
        RegCloseKey(reg);
        return true;
    }
    std::fill(val, val + SZ, 0);
    len = SZ;
    RegQueryValueEx(reg, TEXT("SkinPath"), NULL, NULL, (LPBYTE)&val, &len);
    bmp.ReadFromFile(val);
    RegCloseKey(reg);
    if ((board = construct({0, 1, 2, 3, 4, 5, 6, 7, 8}, bmp, SPosition(0, 0), 16, 16, 16)) == nullptr)
        return false;
    if ((mine = construct({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, bmp, SPosition(0, 33), 11, 21, 12)) == nullptr)
        return false;
    if ((face = construct({0, 2, 3}, bmp, SPosition(0, 55), 26, 26, 27)) == nullptr)
        return false;
    return true;
}

void init_game()
{
    do
    {
        Sleep(1000);
        hwnd = FindWindow(NULL, TEXT("Minesweeper X"));
    } while (hwnd == 0);
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    SetForegroundWindow(hwnd);
    GetWindowRect(hwnd, &win);
    st = 0;
    m = (win.bottom - win.top - 116) / 16;
    n = (win.right - win.left - 30) / 16;
    min = read(SPosition(win.left + 21, win.top + 63), mine) * 100 
        + read(SPosition(win.left + 34, win.top + 63), mine) * 10 
        + read(SPosition(win.left + 47, win.top + 63), mine);
}
} // namespace NHandle