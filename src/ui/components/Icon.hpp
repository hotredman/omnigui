#pragma once

#include <imgui.h>
#include <string>
#include <string_view>
#include <cstdint>

// Универсальный класс пиктограммы (Value Object).
// Объединяет встроенный типовой каталог векторных пиктограмм дизайн-системы
// и поддержку внешних иконок из файлов и ассетов без дублирования деклараций.
class Icon {
public:
    // Каталог встроенных пиктограмм дизайн-системы
    enum Id : uint8_t {
        None = 0,

        // Сайдбар и общие (типовые имена по физической форме)
        Database,        // Стек цилиндрических дисков БД (Архив)
        List,            // Список строк с точками-буллетами (Журнал событий)
        Cog,             // Шестерня машины со спицами (Параметры машины)
        Target,          // Мишень с перекрестием (Калибровка датчиков)
        Gamepad,         // Корпус геймпада (Пульт управления)
        Gear,            // Инженерная шестеренка 6 зубцов (Установки / Настройки)
        LineChart,       // Документ с ломаной линией диаграммы (Обработка результатов)
        Clipboard,       // Планшет с зажимом-клипсой (Паспорт образца)
        BarChart,        // Столбчатая гистограмма (Серия испытаний)

        // Пульт и управление приводом (Remote Control)
        Play,            // Залитый треугольник вправо («ПУСК»)
        Square,          // Залитый правильный квадрат («СТОП»)
        Pause,           // Две параллельные вертикальные полосы («Пауза»)
        RotateCcw,       // Изогнутая стрелка возврата против часовой («Возврат»)
        ArrowDownToLine, // Стрелка вниз в лоток/платформу («Разгрузка»)
        RefreshCw,       // Круговая стрелка обновления («Сброс аварии»)
        ArrowUp,         // Треугольная стрелка вверх («ВВЕРХ»)
        ArrowDown,       // Треугольная стрелка вниз («ВНИЗ»)
        ChevronsUp,      // Сдвоенные стрелки вверх (Ускоренно вверх)
        ChevronsDown,    // Сдвоенные стрелки вниз (Ускоренно вниз)

        // Навигация и прокрутка (Navigation / Carousel / Scrolling)
        ChevronLeft,     // Шеврон влево (<) для горизонтальной прокрутки каруселей/табов
        ChevronRight,    // Шеврон вправо (>) для горизонтальной прокрутки каруселей/табов
        ArrowLeft,       // Стрелка влево (псевдоним ChevronLeft)
        ArrowRight,      // Стрелка вправо (псевдоним ChevronRight)

        // Системные и шапка (Header / Controls)
        Power,           // Круг с вертикальной чертой сверху (Включение питания)
        Maximize,        // 4 раздвигающихся угла (Полный экран)
        Minimize,        // 4 сходящихся угла (Оконный режим)
        Sun,             // Солнце с лучами (Светлая тема)
        Moon,            // Серп луны (Тёмная тема)
        Zero,            // Мишень обнуления с точкой в центре (Тарирование)
        Check,           // Галочка выбора
        Pencil,          // Карандаш / редактирование по месту
        Plus,            // Плюс / Создать / Добавить
        Close,           // Крестик закрытия / очистки / сброса (X)
        Aa,              // Две буквы Aa (Масштаб интерфейса / Шрифты)

        // Действия с данными и ПЛК (Machine Actions)
        Upload,          // Стрелка вверх («Записать» в ПЛК / Выгрузка)
        Download,        // Стрелка вниз («Прочитать» из ПЛК / Загрузка)
        Refresh,         // Круговая стрелка («Заводские» / Сброс аварии / Обновление)

        // Архив и файлы (Archive / File Actions)
        Folder,          // Папка (Открыть папку данных)
        Trash,           // Корзина (Удалить проект)
        Copy,            // Два документа (Дублировать проект)
        ExternalLink,    // Стрелка наружу (Открыть проект)
    };

    enum class SourceType : uint8_t {
        None = 0,
        Builtin,
        Asset,
        File
    };

    // Конструкторы
    Icon() noexcept = default;
    Icon(Id id) noexcept
        : m_type(id == None ? SourceType::None : SourceType::Builtin)
        , m_id(id)
        , m_tintable(true)
    {}

    // Фабричные методы для внешних файлов и ассетов
    static Icon File(std::string_view filePath, bool tintable = true) {
        if (filePath.empty()) return Icon();
        return Icon(SourceType::File, filePath, tintable);
    }
    static Icon file(std::string_view filePath, bool tintable = true) {
        return File(filePath, tintable);
    }

    static Icon Asset(std::string_view assetPath, bool tintable = true) {
        if (assetPath.empty()) return Icon();
        return Icon(SourceType::Asset, assetPath, tintable);
    }
    static Icon asset(std::string_view assetPath, bool tintable = true) {
        return Asset(assetPath, tintable);
    }

    // Состояние и тип
    bool IsEmpty() const noexcept { return m_type == SourceType::None; }
    bool IsValid() const noexcept { return m_type != SourceType::None; }
    explicit operator bool() const noexcept { return IsValid(); }

    SourceType GetType() const noexcept { return m_type; }
    Id GetId() const noexcept { return m_id; }
    const std::string& GetPath() const noexcept { return m_path; }
    bool IsTintable() const noexcept { return m_tintable; }

    // Операторы сравнения с Id (например: icon == Icon::None или icon != Icon::Play)
    bool operator==(Id id) const noexcept {
        if (id == None) return m_type == SourceType::None;
        return m_type == SourceType::Builtin && m_id == id;
    }
    bool operator!=(Id id) const noexcept {
        return !(*this == id);
    }
    friend bool operator==(Id id, const Icon& icon) noexcept {
        return icon == id;
    }
    friend bool operator!=(Id id, const Icon& icon) noexcept {
        return icon != id;
    }

    // Операторы сравнения между объектами Icon
    bool operator==(const Icon& other) const noexcept {
        if (m_type != other.m_type) return false;
        if (m_type == SourceType::None) return true;
        if (m_type == SourceType::Builtin) return m_id == other.m_id;
        return m_path == other.m_path && m_tintable == other.m_tintable;
    }
    bool operator!=(const Icon& other) const noexcept {
        return !(*this == other);
    }

    // Методы отрисовки экземпляра
    void Draw(ImDrawList* dl, ImVec2 center, float size, ImU32 color) const;
    void DrawAt(ImDrawList* dl, ImVec2 pos, float size, ImU32 color) const;
    void Render(float size, ImU32 color) const;

    // Статические утилиты отрисовки
    static void Draw(const Icon& icon, ImDrawList* dl, ImVec2 center, float size, ImU32 color) {
        icon.Draw(dl, center, size, color);
    }
    static void DrawAt(const Icon& icon, ImDrawList* dl, ImVec2 pos, float size, ImU32 color) {
        icon.DrawAt(dl, pos, size, color);
    }
    static void Render(const Icon& icon, float size, ImU32 color) {
        icon.Render(size, color);
    }

private:
    Icon(SourceType type, std::string_view path, bool tintable)
        : m_type(type)
        , m_id(None)
        , m_tintable(tintable)
        , m_path(path)
    {}

    SourceType m_type = SourceType::None;
    Id m_id = None;
    bool m_tintable = true;
    std::string m_path;
};

// Псевдоним типа для удобства
using IconId = Icon::Id;
