#ifndef _LIGHTS_H_
#define _LIGHTS_H_

#include <Arduino.h>
#include <GyverRGB.h>

/*
    Класс подсветки, следит за температурой и нагрузкой видеокарты и процессора.
    При запуске включается выбранный режим подсветки, затем в зависимости от режима работы
    цвета становятся более яркими(холодные цвета перестают использоваться).

    Цвета ленты напрямую зависят от температуры, так как используется "map"

    Первый и основной режим подсветки - это изменение цвета в зависимости от температуры.
    Второй режим подсветки - это радуга которая при повышении температуры,
    уходит в белые тона.
    Третий режим - это измение оттенка красного цвета,
    при повышении температуры.
*/

class Lights {
    public: 
        Lights(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin);

        typedef enum { // Режимы подсветки
            lmColor, // Переключение цветов
            lmRainbow, // Радуга
            lmKelvin // Температура
        } LightsMode;
        
        void update(uint8_t cpu_temp, uint8_t gpu_temp);
        void off() {is_on_ = false;}; // Выключение ленты
        void on() {is_on_ = true;}; // Включение ленты
        void setBrightness(uint8_t bright) {leds_.setBrightness(bright);};
        void nextMode();
        void prevMode();
        void setMode(uint8_t index);
        void setSpeed(uint8_t speed) {speed_ = speed;};
        //String getTempMode(); // Получение режим температуры
        String getModeName() {return mode_name_[lights_mode_];}; // Получение режима подцветки

    private:
        uint8_t red_pin_;
        uint8_t green_pin_;
        uint8_t blue_pin_;

        uint8_t cpu_temp_;
        uint8_t gpu_temp_;
        
        bool is_on_;
        bool is_alarm_timer_;
        uint8_t hsvColor_;
        uint8_t red_;
        uint8_t green_;
        uint8_t blue_;
        uint8_t speed_;

        String mode_name_[3] = {"Color", "Rainbow", "Kelvin"};

        LightsMode lights_mode_; // Основные режимы работы подсветки
        GRGB leds_;
};


#endif // _LIGHTS_H_
