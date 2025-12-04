// esperoysi.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX

#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "raylib.h"
#include "serialib.h"

#include <windows.h>


//  UTILS

static inline std::string trim(const std::string& s) {
    auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}



//  CLASS: SerialPortManager

class SerialPortManager {
private:
    serialib port;
    std::string selectedPort;

public:
    bool selectPort() {
        InitWindow(600, 400, "Seleccionar Puerto COM");
        SetTargetFPS(144);

        Rectangle btn6 = { 50, 100, 200, 50 };
        Rectangle btn7 = { 50, 180, 200, 50 };
        Rectangle btn8 = { 50, 260, 200, 50 };

        while (!WindowShouldClose() && selectedPort == "") {
            BeginDrawing();
            ClearBackground(DARKGRAY);

            DrawText("Selecciona un COM:", 50, 40, 25, RAYWHITE);

            if (drawButton(btn6, "COM6")) selectedPort = "COM6";
            if (drawButton(btn7, "COM7")) selectedPort = "COM7";
            if (drawButton(btn8, "COM8")) selectedPort = "COM8";

            EndDrawing();
        }

        CloseWindow();
        return selectedPort != "";
    }

    bool open() {
        return port.openDevice(
            selectedPort.c_str(), 9600,
            SERIAL_DATABITS_8, SERIAL_PARITY_NONE, SERIAL_STOPBITS_1
        ) == 1;
    }

    std::string readLine() {
        std::string line = "";
        char c;

        while (true) {
            int r = port.readChar(&c, 200);
            if (r == 1) {
                if (c == '\n') break;
                if (c != '\r') line += c;
            }
            else break;
        }
        return trim(line);
    }

    void writeChar(char c) { port.writeChar(c); }
    void close() { port.closeDevice(); }

private:
    bool drawButton(Rectangle rec, const char* text) {
        DrawRectangleRec(rec, LIGHTGRAY);
        DrawText(text, rec.x + 70, rec.y + 15, 20, BLACK);

        if (CheckCollisionPointRec(GetMousePosition(), rec) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            return true;

        return false;
    }
};



//  CLASS: Character (Bob Esponja)

class Character {
public:
    int x, y;
    int size;
    const int MIN_X = 100;
    const int MAX_X = 1400;

    Character(int screenW, int screenH)
    {
        size = 150;
        x = (screenW - size) / 2;
        y = (screenH - size) / 2;
    }

    void moveLeft() {
        x -= 5;
        if (x < MIN_X) x = MIN_X;
    }

    void moveRight() {
        x += 5;
        if (x > MAX_X) x = MAX_X;
    }

    void draw() {
        // Bob Esponja 
        DrawRectangle(x, y - 80, size + 40, size + 40, YELLOW);
        DrawRectangleLines(x, y - 80, size + 40, size + 40, BLACK);

        DrawRectangle(x, y + 80, size + 40, size - 80, BROWN);
        DrawRectangleLines(x, y + 80, size + 40, size - 80, BLACK);

        DrawRectangle(x, y + 60, size + 40, 20, WHITE);
        DrawRectangleLines(x, y + 60, size + 40, 20, BLACK);

        int cx = x + (size + 40) / 2;
        DrawRectangle(cx - 6, y + 60, 12, 35, RED);
        DrawRectangleLines(cx - 6, y + 60, 12, 35, BLACK);

        int eyeY = y - 25;
        int eyeLeftX = cx - 28;
        int eyeRightX = cx + 28;

        DrawCircle(eyeLeftX, eyeY, 28, WHITE);
        DrawCircle(eyeRightX, eyeY, 28, WHITE);
        DrawCircleLines(eyeLeftX, eyeY, 28, BLACK);
        DrawCircleLines(eyeRightX, eyeY, 28, BLACK);

        DrawCircle(eyeLeftX, eyeY, 12, SKYBLUE);
        DrawCircle(eyeRightX, eyeY, 12, SKYBLUE);

        DrawCircle(eyeLeftX, eyeY, 5, BLACK);
        DrawCircle(eyeRightX, eyeY, 5, BLACK);

        DrawCircle(x + 25, y - 5, 10, PINK);
        DrawCircle(x + size + 15, y - 5, 10, PINK);
    }
};



//  CLASS: TemperatureController

class TemperatureController {
public:
    float currentTemp = 32;
    float lastTemp = 32;

    bool active = false;

    Sound lowSound;
    Sound highSound;

    bool playLow = false;
    bool playHigh = false;

    void initSounds() {
        InitAudioDevice();
        lowSound = LoadSound("tubo cayendo.wav");
        highSound = LoadSound("YOUR PHONE LINGING (sound effect).wav");
    }

    void unloadSounds() {
        UnloadSound(lowSound);
        UnloadSound(highSound);
        CloseAudioDevice();
    }

    void updateFromSerial(const std::string& line) {
        if (line.rfind("A:", 0) == 0)
            active = std::stoi(line.substr(2)) == 1;

        else if (line.rfind("T:", 0) == 0 && active) {
            lastTemp = currentTemp;
            currentTemp = std::stof(line.substr(2));
        }
    }

    void applyMovement(Character& c) {
        if (!active) return;

        if (currentTemp > lastTemp) c.moveRight();
        if (currentTemp < lastTemp) c.moveLeft();
    }

    void playAlerts() {
        if (!active) return;

        if (currentTemp < 25) {
            if (!playLow) {
                PlaySound(lowSound);
                playLow = true;
                playHigh = false;
            }
        }
        else if (currentTemp > 32) {
            if (!playHigh) {
                PlaySound(highSound);
                playHigh = true;
                playLow = false;
            }
        }
        else {
            playLow = playHigh = false;
        }
    }
};



//  CLASS: UIManager

class UIManager {
public:
    Rectangle toggleBtn{ 50, 160, 200, 50 };

    void drawSensor() {
        DrawText("Sensor DTH-11", 50, 40, 25, BLACK);

        int sx = 50;
        int sy = 325 - 60;

        DrawRectangle(sx + 30, sy - 30, 100, 150, BLUE);
        DrawRectangleLines(sx + 30, sy - 30, 100, 150, RAYWHITE);

        DrawRectangle(sx + 60, sy + 5, 40, 20, GRAY);
        DrawRectangle(sx + 50, sy + 80, 60, 20, BLACK);
        DrawRectangleLines(sx + 50, sy + 80, 60, 20, RAYWHITE);
    }

    bool toggleButton(bool active) {
        Color c = active ? GREEN : RED;
        const char* t = active ? "ACTIVADO" : "DESACTIVADO";

        DrawRectangleRec(toggleBtn, c);
        DrawText(t, toggleBtn.x + 35, toggleBtn.y + 15, 20, BLACK);

        return CheckCollisionPointRec(GetMousePosition(), toggleBtn) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    }
};



//  CLASS: MainApp

class MainApp {
public:
    SerialPortManager serial;
    TemperatureController tempCtrl;
    UIManager ui;
    Character* character;

    const int W = 1600, H = 650;

    void run() {
        if (!serial.selectPort()) return;
        if (!serial.open()) return;

        InitWindow(W, H, "Control por Temperatura");
        SetTargetFPS(144);

        character = new Character(W, H);
        tempCtrl.initSounds();

        while (!WindowShouldClose()) {

            std::string line = serial.readLine();
            if (!line.empty())
                tempCtrl.updateFromSerial(line);

            tempCtrl.applyMovement(*character);
            tempCtrl.playAlerts();

            if (ui.toggleButton(tempCtrl.active)) {
                tempCtrl.active = !tempCtrl.active;
                serial.writeChar(tempCtrl.active ? '1' : '0');
            }

            BeginDrawing();
            ClearBackground(SKYBLUE);

            ui.drawSensor();
            character->draw();

            DrawText(TextFormat("Temperatura: %.1f C", tempCtrl.currentTemp),
                50, 100, 30, BLACK);

            DrawText(TextFormat("Posicion X: %d px", character->x),
                50, 140, 30, BLACK);

            EndDrawing();
        }

        tempCtrl.unloadSounds();
        serial.close();
        CloseWindow();
    }
};



//  MAIN

int main() {
    MainApp app;
    app.run();
    return 0;
}
















