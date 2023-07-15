#include <iostream>
#include <Windows.h>
#include <string>
#include <chrono>
#include <map>
#include <vector>
#include <algorithm>

int nScreenWidth = 120;
int nScreenHeight = 40;

float playerX = 8.0f;
float playerY = 8.0f;
float playerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float FOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main()
{
    // Create screen buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    std::wstring map;

    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.........#....#";
    map += L"#.........#....#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#........#######";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = std::chrono::system_clock::now();

    while (true)
    {

        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapseTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapseTime.count();


         if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
        {
             playerA += (0.1f);
        }

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
        {
            playerA -= (0.1f);
        }
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            playerX = playerX + sinf(playerA) * 5.0f * fElapsedTime;
            playerY = playerY + cosf(playerA) * 5.0f * fElapsedTime;

            if (map[(int)playerY * nMapWidth + (int)playerX] == '#')
            {
                playerX = playerX - sinf(playerA) * 5.0f * fElapsedTime;
                playerY = playerY - sinf(playerA) * 5.0f * fElapsedTime;
            }
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            playerX = playerX - sinf(playerA) * 5.0f * fElapsedTime;
            playerY = playerY - cosf(playerA) * 5.0f * fElapsedTime;

            if (map[(int)playerY * nMapWidth + (int)playerX] == '#')
            {
                playerX = playerX + sinf(playerA) * 5.0f * fElapsedTime;
                playerY = playerY + sinf(playerA) * 5.0f * fElapsedTime;
            }
        }




        for (int x = 0; x < nScreenWidth; x++)
        {
            float rayAngle = (playerA + FOV / 2.0f) - ((float)x / (float)nScreenWidth) * FOV;

            float distanceToWall = 0;
            bool hitWall = false;
            bool boundary = false;

            float EyeX = sinf(rayAngle);
            float EyeY = cosf(rayAngle);

            while (!hitWall && distanceToWall < fDepth)
            {
                distanceToWall += 0.1f;

                int nTestX = (int)(playerX + EyeX * distanceToWall);
                int nTestY = (int)(playerY + EyeY * distanceToWall);

                // Test if ray is out
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    hitWall = true; // Just set the distance to max
                    distanceToWall = fDepth;
                }
                else
                {
                    if (map[nTestY * nMapWidth + nTestX] == '#')
                    {
                        hitWall = true;


                        std::vector<std::pair<float, float>> p;

                        for (int tx = 0; tx < 3; tx++)
                        {
                            for (int ty = 0; ty < 3; ty++)
                            {
                                float vy = (float)nTestY + ty - playerY;
                                float vx = (float)nTestX + tx - playerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (EyeX * vx / d) + (EyeY * vy / d);
                                p.push_back(std::make_pair(d, dot));
                            }

                            std::sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; });
                            

                            float fBound = 0.005;
                            if (acos(p.at(0).second) < fBound) boundary = true;
                            if (acos(p.at(1).second) < fBound) boundary = true;
                            if (acos(p.at(2).second) < fBound) boundary = true;
                        }
                    }
                }
            }

            // Calculate the distance to floor and ceiling
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)distanceToWall);
            int nFloor = nScreenHeight - nCeiling;


            short nShade = ' ';
            if (distanceToWall <= fDepth / 4.0f)        nShade = 0x2588;
            else if (distanceToWall < fDepth / 3.0f)    nShade = 0x2593;
            else if (distanceToWall < fDepth / 2.0f)    nShade = 0x2592;
            else if (distanceToWall < fDepth)           nShade = 0x2591;
            else                                        nShade = ' ';
           
            if (boundary)                                nShade = ' ';

            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y < nCeiling)
                {
                    screen[y * nScreenWidth + x] = ' ';
                }
                else if (y > nCeiling && y <= nFloor)
                {
                    screen[y * nScreenWidth + x] = nShade;
                }
                else
                {
                    short bShade = ' ';
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)           bShade = '#';
                    else if (b < 0.5)       bShade = 'X';
                    else if (b < 0.75)      bShade = '.';
                    else if (b < 0.9)       bShade = '-';
                    else                    bShade = ' ';
                    screen[y * nScreenWidth + x] = bShade;
                }
            }
        }

        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", playerX, playerY, playerA, 0.1f / fElapsedTime);

        

        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
        Sleep(16);
    }

    delete[] screen;

    return 0;
}
