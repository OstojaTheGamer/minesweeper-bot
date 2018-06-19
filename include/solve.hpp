#pragma once
#include "declare.hpp"
#include "handle.hpp"

namespace NSolve
{
struct SComponent
{
    std::vector<SPosition> ve;

    bool valid(int mask)
    {
        bool ret = true;
        for (unsigned int i = 0; i < ve.size(); i++)
            num[ve[i].x][ve[i].y] = 9 + (mask >> i & 1);
        for (SPosition &u : ve)
            for (int i = 0; i < 8; i++)
                ret &= SPosition(u.x + DX[i], u.y + DY[i]).valid();
        for (SPosition &u : ve)
            num[u.x][u.y] = -1;
        return ret;
    }
};

// int read(SPosition u)
// {
//     int x;
//     std::cout << u.x << " " << u.y << ": ";
//     std::cin >> x;
//     return x;
// }

// int status()
// {
//     int x;
//     std::cout << "status: ";
//     std::cin >> x;
//     return x;
// }

// void click(SPosition u)
// {
//     std::cout << "click: " << u.x << " " << u.y << std::endl;
// }

void open(SPosition u)
{
    if (num[u.x][u.y] == -1 && (num[u.x][u.y] = NHandle::read(u.pixel_pos(), NHandle::board)) == 0)
        for (int i = 0; i < 8; i++)
            if (SPosition(u.x + DX[i], u.y + DY[i]).on_board() && num[u.x + DX[i]][u.y + DY[i]] == -1)
                open(SPosition(u.x + DX[i], u.y + DY[i]));
}

void naive();
void tanker();
void probability();

void solve()
{
    int st;
    for (int x = 0; x < m; x++)
        for (int y = 0; y < n; y++)
            num[x][y] = -1;
    for (int x = 0; x < m; x++)
        for (int y = 0; y < n; y++)
            if (num[x][y] == -1)
            {
                NHandle::click(SPosition(x, y).pixel_pos());
                st = NHandle::status();
                if (st == 2)
                {
                    std::cout << "Fail\n";
                    return;
                }
                else if (st == 3)
                {
                    std::cout << "Finish\n";
                    return;
                }
                else
                {
                    open(SPosition(x, y));  
                    naive();
                    st = NHandle::status();
                    if (st == 2)
                    {
                        std::cout << "Fail\n";
                        return;
                    }
                    else if (st == 3)
                    {
                        std::cout << "Finish\n";
                        return;
                    }
                }
            }
    std::cout << "Finish\n";
    return;
}

void naive()
{
    bool border = false;
    for (int x = 0; x < m; x++)
        for (int y = 0; y < n; y++)
            if (SPosition(x, y).border_valid())
            {
                border = true;
                SPosition u = SPosition(x, y);
                bool move = false;
                SComponent cur;
                for (int i = 0; i < 8; i++)
                    if (SPosition(u.x + DX[i], u.y + DY[i]).on_board() && num[u.x + DX[i]][u.y + DY[i]] == -1)
                        cur.ve.push_back(SPosition(u.x + DX[i], u.y + DY[i]));
                int yes = (1 << cur.ve.size()) - 1, no = 0;
                for (int mask = 0; mask < (1 << cur.ve.size()); mask++)
                    if (cur.valid(mask))
                    {
                        yes &= mask;
                        no |= mask;
                    }
                for (unsigned int i = 0; i < cur.ve.size(); i++)
                {
                    if (yes >> i & 1)
                    {
                        move = true;
                        num[cur.ve[i].x][cur.ve[i].y] = 10;
                    }
                    if (!(no >> i & 1))
                    {
                        move = true;
                        NHandle::click(cur.ve[i].pixel_pos());
                        int st = NHandle::status();
                        if (st == 2)
                        {
                            std::cout << "Fail\n";
                            return;
                        }
                        else if (st == 3)
                        {
                            std::cout << "Finish\n";
                            return;
                        }
                        open(cur.ve[i]);
                    }
                }
                if (move)
                {
                    naive();
                    return;
                }
            }
    if (border)
        tanker();
}

void tanker()
{
    
}
} // namespace NSolve