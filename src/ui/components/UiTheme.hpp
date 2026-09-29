#pragma once

#include <imgui.h>
#include <string>
#include "ui/components/ThemePalette.hpp"
#include "ui/components/charts/ChartStyle.hpp"

// Цвета стилей компонентов (поля col*) не задаются константами: их собирает
// UiTheme::ApplyPalette из семантики палитры текущей темы (ThemePalette)

// ============================================================================
// 1. Семантические варианты компонентов дизайн-системы (Semantic Variants)
// ============================================================================
enum class UiVariant {
    Default,    // Нейтральный контрол / обычный текст
    Primary,    // Основное действие / бренд-акцент (синий)
    Secondary,  // Второстепенное нейтральное (серо-синий)
    Success,    // Успех, норма, ПУСК (зеленый)
    Warning,    // Предупреждение, внимание (янтарный / оранжевый)
    Danger,     // Ошибка, авария, СТОП (красный)
    Info        // Информационный / циан акцент
};

// ============================================================================
// 1.1. Стандартная размерная шкала интерактивных элементов (T-Shirt Scale)
// ============================================================================
enum class UiSize {
    Large,   // 48px — Пульт управления / Touch (ПУСК/СТОП/Траверса)
    Medium,  // 36px — Формы, карточки, диалоговые окна (Default)
    Small,   // 30px — Тулбары, фильтры, строка поиска
    Mini     // 26px — Строки таблиц, инлайн-действия
};

// Метрики контрола под конкретный размер с автоматическим масштабированием DPI
struct ControlMetrics {
    float height    = 36.0f; // Высота фрейма (уже масштабирована под scale)
    float paddingX  = 12.0f; // Горизонтальный паддинг
    float paddingY  = 6.0f;  // Вертикальный паддинг
    float rounding  = 6.0f;  // Скругление рамки
    float iconSize  = 16.0f; // Размер векторной иконки
    ImFont* font    = nullptr;// Рекомендуемый шрифт
    float fontSize  = 18.0f; // Базовый размер шрифта в pt
};

// Дескриптор стиля семантического варианта (цвета фона, текста, рамок)
struct SemanticStyle {
    ImU32  colText;     // Цвет текста (для меток, заголовков, иконок)
    ImU32  colBtnText;  // Цвет текста на залитой кнопке (обычно белый)
    ImU32  colBorder;   // Цвет рамки (для InputField, Tag, Card)
    ImVec4 colBg;       // Фон кнопки / плашки
    ImVec4 colBgHover;  // Фон кнопки при наведении
    ImVec4 colBgActive; // Фон кнопки при нажатии
};

// ============================================================================
// 2. Дескрипторы стилей компонентов (Component Styles)
// ============================================================================

// Стиль панели вкладок (TabBar)
struct TabBarStyle {
    ImFont* font           = nullptr; // Шрифт вкладок
    float fontSize         = 20.0f;   // Базовый размер шрифта вкладок (20pt Medium)
    float height           = 38.0f;   // Базовая высота вкладки (компактная, без лишнего воздуха)
    float itemSpacing      = 6.0f;    // Базовый зазор между вкладками по горизонтали
    float separatorGapTop  = 4.0f;    // Зазор от низа табов до линии разделителя
    float separatorGapBottom = 6.0f;  // Зазор от линии разделителя до контента
    float cornerRadius     = 6.0f;    // Радиус скругления активной вкладки
    float horizontalPadding = 14.0f;  // Боковой отступ внутри вкладки

    ImU32 colActiveBg      = 0;
    ImU32 colActiveText    = 0;
    ImU32 colInactiveText  = 0;
    ImU32 colHoverBg       = 0;
    ImU32 colHoverText     = 0;
    ImU32 colSeparator     = 0;
};

// Стиль карточки (Card)
struct CardStyle {
    ImFont* titleFont      = nullptr; // Шрифт заголовка
    float titleFontSize    = 22.0f;   // Базовый размер шрифта заголовка (22pt Bold)
    float titleSpacing     = 10.0f;   // Отступ снизу от заголовка до полей
    float paddingX         = 16.0f;   // Горизонтальный внутренний отступ
    float paddingY         = 14.0f;   // Вертикальный внутренний отступ
    float rowSpacing       = 8.0f;    // Межстрочный зазор между рядами полей
    float columnSpacing    = 12.0f;   // Зазор между колонками в 12-колоночной сетке
    float cornerRadius     = 8.0f;    // Радиус скругления карточки
    float borderSize       = 1.0f;    // Толщина рамки

    ImU32 colBg            = 0;
    ImU32 colBorder        = 0;
    ImU32 colTitle         = 0;
};

// Стиль поля ввода (InputField)
struct InputFieldStyle {
    ImFont* labelFont      = nullptr; // Шрифт подписи
    float labelFontSize    = 18.0f;   // Базовый размер подписи (18pt Regular)
    float labelSpacing     = 4.0f;    // Зазор от подписи до рамки ввода

    ImFont* valueFont      = nullptr; // Шрифт вводимого значения
    float valueFontSize    = 20.0f;   // Базовый размер значения (20pt Medium)

    float framePaddingX    = 10.0f;   // Внутренний горизонтальный отступ
    float framePaddingY    = 6.0f;    // Внутренний вертикальный отступ
    float frameRounding    = 6.0f;    // Радиус скругления поля
    float borderSize       = 1.0f;    // Толщина рамки

    ImFont* unitFont       = nullptr; // Шрифт единицы измерения
    float unitFontSize     = 18.0f;   // Базовый размер единиц (18pt Regular)

    ImU32 colLabel         = 0;
    ImU32 colBg            = 0;
    ImU32 colBorder        = 0;
    ImU32 colText          = 0;
    ImU32 colUnit          = 0;
};

// Стиль выпадающего списка (Combo / Select)
struct ComboStyle {
    ImFont* labelFont          = nullptr; // Шрифт подписи
    float labelFontSize        = 18.0f;   // Базовый размер подписи (18pt Regular)
    float labelSpacing         = 4.0f;    // Зазор от подписи до рамки поля

    ImFont* font               = nullptr; // Шрифт выбранного значения
    float fontSize             = 20.0f;   // Базовый размер шрифта значения (20pt Medium)

    float framePaddingX        = 10.0f;   // Внутренний горизонтальный отступ
    float framePaddingY        = 6.0f;    // Внутренний вертикальный отступ
    float frameRounding        = 6.0f;    // Радиус скругления поля
    float borderSize           = 1.0f;    // Толщина рамки

    // Стрелочка-шеврон
    float arrowWidth           = 7.0f;    // Базовая ширина стрелочки
    float arrowHeight          = 4.5f;    // Базовая высота стрелочки
    float arrowPaddingRight    = 12.0f;   // Отступ стрелочки от правого края

    // Выпадающий список (Popup)
    float popupRounding        = 6.0f;    // Радиус скругления всплывающего окна
    float popupBorderSize      = 1.0f;    // Толщина рамки всплывающего окна

    // Цветовая палитра
    ImU32 colLabel             = 0;
    ImU32 colBg                = 0;
    ImU32 colBgHovered         = 0;
    ImU32 colBgActive          = 0;
    ImU32 colBorder            = 0;
    ImU32 colBorderHovered     = 0;
    ImU32 colText              = 0;
    ImU32 colArrow             = 0; // Шеврон
    ImU32 colArrowHovered      = 0; // Ярче при наведении
    ImU32 colPopupBg           = 0;
    ImU32 colPopupBorder       = 0;
};

// Стиль переключателя (Toggle)
struct ToggleStyle {
    float switchWidth          = 42.0f;   // Ширина овального тумблера
    float switchHeight         = 22.0f;   // Высота овального тумблера
    float knobPadding          = 2.0f;    // Зазор бегунка от краев тумблера
    float minHeight            = 42.0f;   // Минимальная высота всей строки контрола
    float labelSpacing         = 10.0f;   // Отступ между тумблером и текстом
    float textLineSpacing      = 2.0f;    // Отступ между заголовком и подзаголовком

    ImFont* fontLabel          = nullptr; // Шрифт основного текста
    ImFont* fontSublabel       = nullptr; // Шрифт подзаголовка
    float fontSizeLabel        = 20.0f;   // Размер шрифта основного текста (20pt Medium)
    float fontSizeSublabel     = 18.0f;   // Размер шрифта подзаголовка (18pt Regular)

    // Цвета
    ImU32 colTrackOff          = 0;    // Фон выключенного трека
    ImU32 colTrackOffHover     = 0;    // Фон выключенного трека при наведении
    ImU32 colTrackOffBorder    = 0;    // Рамка выключенного трека
    ImU32 colTrackOn           = 0;  // Фон включенного трека
    ImU32 colTrackOnHover      = 0;  // Фон включенного трека при наведении
    ImU32 colTrackOnBorder     = 0;  // Рамка включенного трека
    ImU32 colKnobOff           = 0; // Бегунок выключен
    ImU32 colKnobOn            = 0; // Бегунок включен

    ImU32 colTextMain          = 0; // Основной заголовок
    ImU32 colTextSub           = 0; // Подзаголовок / описание
};

// Стиль бокового меню (Sidebar)
struct SidebarStyle {
    float width            = 230.0f;  // Базовая ширина боковой панели
    float itemHeight = 40.0f;         // Базовая высота пункта
    float itemSpacing = 2.0f;         // Отступ между пунктами
    float cornerRadius     = 7.0f;    // Скругление плашки пункта
    float activeBarWidth   = 4.0f;    // Ширина синей вертикальной полоски активного пункта
    float iconSize         = 20.0f;   // Размер векторных иконок

    ImFont* font           = nullptr; // Обычный шрифт
    ImFont* activeFont     = nullptr; // Активный шрифт
    float fontSize         = 20.0f;   // Базовый размер шрифта сайдбара (20pt Medium)

    ImU32 colBg            = 0;
    ImU32 colActiveBg      = 0;
    ImU32 colHoverBg       = 0;
    ImU32 colActiveText    = 0;
    ImU32 colInactiveText  = 0;
    ImU32 colHoverText     = 0;
    ImU32 colActiveBar     = 0;
    ImU32 colSeparator     = 0;
    ImU32 colScrollRegionBg = 0; // Подложка прокручиваемого блока (список испытаний)
};

// Стиль индикатора (Indicator)
struct IndicatorStyle {
    bool autoWidth         = true;    // Автоматическая ширина по содержимому
    float minWidth         = 110.0f;  // Минимальная ширина плашки
    float width            = 180.0f;  // Фиксированная/базовая ширина (если autoWidth == false)
    int defaultDigits      = 5;       // Знаков по умолчанию (например, "99.99")

    float height           = 56.0f;   // Базовая высота плашки
    float cornerRadius     = 6.0f;    // Скругление
    float padX             = 10.0f;   // Внутренний отступ X
    float padY             = 3.0f;    // Внутренний отступ Y

    ImFont* titleFont      = nullptr; // Шрифт подписи
    float titleFontSize    = 16.0f;

    ImFont* valueFont      = nullptr; // Шрифт значения
    float valueFontSize    = 40.0f;

    ImFont* unitFont       = nullptr; // Шрифт единицы
    float unitFontSize     = 20.0f;

    // Стили квадратной кнопки тарирования (Tare button)
    float tareBtnSize        = 26.0f;   // Сторона квадратной кнопки
    float tareBtnRadius      = 4.0f;    // Скругление кнопки
    std::string tareLabel    = "";      // Текст кнопки (если пусто - рисуется векторная мишень тарирования)
    ImU32 colTareBtn         = 0;       // Фон кнопки
    ImU32 colTareBtnHover    = 0;      // Фон при наведении
    ImU32 colTareBtnActive   = 0;       // Фон при нажатии
    ImU32 colTareBorder      = 0;      // Рамка кнопки
    ImU32 colTareBorderHover = 0;     // Рамка при наведении
    ImU32 colTareIcon        = 0;    // Символ/иконка
    ImU32 colTareIconHover   = 0;    // Символ/иконка при наведении

    ImU32 colBg            = 0;
    ImU32 colBorder        = 0;
    ImU32 colTitle         = 0;
    ImU32 colValue         = 0;
    ImU32 colUnit          = 0;
};

// Стиль информационного табло / баннера значения (ValueDisplay)
struct ValueDisplayStyle {
    ImFont* valueFont          = nullptr; // Крупный шрифт значения
    float valueFontSize        = 38.0f;   // Базовый размер крупного шрифта значения (>= 60px)
    float valueCompactFontSize = 24.0f;   // Компактный размер шрифта значения (< 60px)

    ImFont* unitFont           = nullptr; // Шрифт единицы измерения
    float unitFontSize         = 18.0f;   // Базовый размер шрифта единиц (>= 60px)
    float unitCompactFontSize  = 16.0f;   // Компактный размер шрифта единиц (< 60px)

    float cornerRadius         = 8.0f;    // Радиус скругления подложки
    float borderSize           = 1.0f;    // Толщина рамки

    ImU32 colBg                = 0; // Тёмный инсетный фон
    ImU32 colBorder            = 0; // Рамка
    ImU32 colValue             = 0; // Яркий текст значения
    ImU32 colUnit              = 0; // Приглушенный текст единиц
};

// Стиль шапки приложения (Header)
struct HeaderStyle {
    float height            = 80.0f;   // Базовая высота шапки
    float paddingX          = 16.0f;   // Горизонтальный внутренний отступ
    float paddingY          = 12.0f;   // Вертикальный внутренний отступ
    float zoneSpacing       = 10.0f;   // Зазор между зонами (Left, Center, Right)
    float separatorSize     = 1.0f;    // Толщина нижней линии-разделителя
    float defaultLeftWidth  = 310.0f;  // Начальная ширина левой зоны до первого замера
    float defaultRightWidth = 84.0f;   // Начальная ширина правой зоны до первого замера

    ImU32 colBg             = 0; // Фон окна шапки
    ImU32 colSeparator      = 0; // Разделитель
};

// Стиль строки состояния внизу окна (StatusBar)
struct StatusBarStyle {
    float height            = 30.0f;   // Базовая высота полосы
    float paddingX          = 12.0f;   // Горизонтальный внутренний отступ
    float itemSpacing       = 12.0f;   // Зазор между элементами и разделителем
    float separatorSize     = 1.0f;    // Толщина верхней линии и разделителей

    ImU32 colBg             = 0;    // Фон полосы (как у шапки)
    ImU32 colTopLine        = 0;    // Верхняя линия-разделитель
    ImU32 colSeparator      = 0;    // Вертикальный разделитель элементов
    ImU32 colText           = 0; // Обычный (вторичный) текст
};

// Стиль виджета статуса устройства / соединения (DeviceStatus)
struct DeviceStatusStyle {
    float width               = 220.0f;  // Базовая ширина плашки
    float height              = 56.0f;   // Базовая высота (согласована с Indicator::height)
    float cornerRadius        = 6.0f;    // Скругление рамки
    float ledRadius           = 5.0f;    // Радиус светодиода
    float borderSize          = 1.0f;    // Толщина рамки
    float actionBtnSize       = 36.0f;   // Размер кнопки действия [P] (увеличен с 28.0f)

    ImFont* titleFont         = nullptr; // Шрифт имени устройства (16.0f)
    float titleFontSize       = 16.0f;
    ImFont* statusFont        = nullptr; // Шрифт статуса (20.0f)
    float statusFontSize      = 20.0f;

    ImU32 colBg               = 0; // Фон бейджа
    ImU32 colBorder           = 0; // Рамка бейджа
    ImU32 colTitle            = 0; // Цвет имени устройства
    ImU32 colStatusText       = 0; // Цвет текста статуса

    // Семантические цвета светодиода (LED) по стандарту IEC/SCADA:
    ImU32 colLedDisconnected  = 0; // Отключен / обесточен
    ImU32 colLedConnecting    = 0;  // Желтый — подключение / рукопожатие
    ImU32 colLedIdle          = 0;   // Зеленый — готов, ожидание команды
    ImU32 colLedRunning       = 0;  // Синий — активный процесс / в работе
    ImU32 colLedPaused        = 0;   // Янтарный — пауза / удержание
    ImU32 colLedFault         = 0;   // Красный — авария / сбой
};

// Стиль контейнера горизонтальной карусели (Carousel)
struct CarouselStyle {
    float arrowWidth        = 20.0f;   // Компактная ширина кнопок со стрелками < и >
    float itemSpacing       = 8.0f;    // Зазор между элементами в карусели
    float scrollSpeed       = 15.0f;   // Скорость плавной интерполяции скролла (Lerp)
    float scrollStep        = 180.0f;  // Базовый шаг скролла при клике на стрелки
    float paddingY          = 0.0f;    // Дополнительный вертикальный зазор
    bool hideDisabledArrows = true;    // Скрывать неактивную стрелку (когда в начале или конце ленты)

    ImU32 colBg             = 0; // Подложка ленты прокрутки
    ImU32 colArrowBg        = 0; // Мягкий чиповый фон стрелок
    ImU32 colArrowText      = 0; // Текст / пиктограмма стрелок
};

// Стиль контекстного меню (ContextMenu)
struct ContextMenuStyle {
    // Типографика (увеличенные читаемые шрифты)
    ImFont* headerFont       = nullptr; // Шрифт заголовка секции (Roboto-Bold)
    float headerFontSize     = 18.0f;   // Крупный акцентный заголовок (18px Bold)

    ImFont* itemFont         = nullptr; // Шрифт пунктов меню (Roboto-Medium)
    float itemFontSize       = 20.0f;   // Крупный читаемый шрифт пунктов (20px Medium)

    // Геометрия
    float cornerRadius       = 10.0f;   // Скругление самого окна попапа
    float itemRounding       = 5.0f;    // Скругление рамки выделения пункта при наведении
    float itemHeight         = 36.0f;   // Высота строки пункта меню
    float checkColWidth      = 28.0f;   // Ширина колонки под галочку
    float minWidth           = 210.0f;  // Минимальная ширина меню
    float windowPaddingX     = 14.0f;   // Горизонтальный паддинг попапа
    float windowPaddingY     = 12.0f;   // Вертикальный паддинг попапа
    float itemSpacingY       = 4.0f;    // Зазор между пунктами

    // Цветовая палитра
    ImU32 colBg              = 0;       // Фон всплывающего окна
    ImU32 colBorder          = 0;       // Контур окна
    ImU32 colHeader          = 0;    // Цвет заголовка (muted caps)
    ImU32 colText            = 0;    // Обычный текст пункта
    ImU32 colTextActive      = 0;     // Активный пункт
    ImU32 colHoverBg         = 0;      // Подложка при наведении
    ImU32 colActiveBg        = 0;       // Подложка при клике
    ImU32 colCheckmark       = 0;     // Цвет галочки
    ImU32 colSeparator       = 0;       // Разделитель
};

// Стиль селектора пресетов со встроенным табло (PresetGrid)
struct PresetGridStyle {
    // Геометрия и компоновка
    float displayHeight       = 38.0f;  // Базовая высота информационного табло
    float displaySpacing      = 0.0f;   // Дополнительный отступ от табло до матрицы кнопок (по умолчанию 0, единый зазор rowSpacing)
    float displayRounding     = 8.0f;   // Радиус скругления табло (гармонирует с Card)

    float btnHeight           = 34.0f;  // Высота кнопок пресетов (аккуратные, соразмерные клавиши)
    float btnRounding         = 6.0f;   // Радиус скругления кнопок пресетов
    float colSpacing          = 6.0f;   // Горизонтальный зазор между кнопками в ряду
    float rowSpacing          = 6.0f;   // Вертикальный зазор между рядами кнопок

    // Типографика
    ImFont* displayValFont    = nullptr;// Крупный шрифт цифр табло
    float displayValFontSize  = 24.0f;  // Соразмерный акцентный шрифт значения на табло
    ImFont* displayUnitFont   = nullptr;// Шрифт единиц измерения табло
    float displayUnitFontSize = 16.0f;

    ImFont* btnFont           = nullptr;// Шрифт пресетов на кнопках
    float btnFontSize = 20.0f;          // Четкий аккуратный шрифт цифр пресетов (16pt Medium)

    // Цветовая палитра табло
    ImU32 colDisplayBg        = 0;   // Тёмный фон табло
    ImU32 colDisplayBorder    = 0;   // Рамка табло
    ImU32 colDisplayValue     = 0;// Яркие цифры значения
    ImU32 colDisplayUnit      = 0;// Приглушённый цвет единицы

    // Цветовая палитра кнопок пресетов
    ImU32 colBtnBg            = 0;   // Фон обычной кнопки
    ImU32 colBtnBorder        = 0;   // Рамка обычной кнопки
    ImU32 colBtnText          = 0;// Текст обычной кнопки

    ImU32 colBtnHoverBg       = 0;   // Фон кнопки при наведении
    ImU32 colBtnHoverBorder   = 0;  // Рамка при наведении
    ImU32 colBtnHoverText     = 0;// Текст при наведении

    ImU32 colBtnActiveBg      = 0; // Фон выбранной кнопки
    ImU32 colBtnActiveBorder  = 0; // Рамка выбранной кнопки
    ImU32 colBtnActiveText    = 0;// Текст выбранной кнопки
};

// Стиль боковой панели (SidePanel)
struct SidePanelStyle {
    float splitterWidth     = 6.0f;                       // Интерактивная ширина сплиттера для захвата мыши
    float splitterLineWidth = 1.0f;                       // Толщина линии разделителя
    ImU32 colBg             = 0;  // Подложка панели (прозрачный — без подложки)
    ImU32 colBorder         = 0;       // Рамка панели (по умолчанию выключена)
    float borderSize        = 0.0f;
    ImVec2 padding          = ImVec2(0.0f, 0.0f);         // Внутренние отступы панели

    ImU32 colSplitter       = 0;  // Линия сплиттера в покое
    ImU32 colSplitterHover  = 0;// Подсветка при наведении
    ImU32 colSplitterActive = 0;// Подсветка при перетаскивании (Accent Blue)
};

// Стиль панели инструментов (Toolbar)
struct ToolbarStyle {
    float height            = 46.0f;                       // Базовая высота тулбара
    float paddingX          = 12.0f;                       // Горизонтальный отступ
    float paddingY          = 7.0f;                        // Вертикальный отступ (центрирует 32px элементы: (46-32)/2 = 7)
    float zoneSpacing       = 10.0f;                       // Зазор между зонами (Left, Center, Right)
    float defaultLeftWidth  = 200.0f;                      // Начальная ширина левой зоны
    float defaultRightWidth = 250.0f;                      // Начальная ширина правой зоны
    float separatorSize     = 1.0f;                        // Толщина нижней линии-разделителя

    float ContentHeight() const { return height - paddingY * 2.0f; }

    ImU32 colBg             = 0;   // Фон панели
    ImU32 colSeparator      = 0;   // Нижняя линия-разделитель
};

// Стиль редактируемой текстовой метки (EditableLabel)
struct EditableLabelStyle {
    float boxHeight         = 32.0f;                       // Базовая высота кнопки и поля ввода
    float iconSize          = 16.0f;                       // Размер иконки карандаша
    float spacing           = 8.0f;                        // Зазор между иконкой и текстом
    float frameRounding     = 6.0f;                        // Скругление рамки поля ввода
    float minInputWidth     = 200.0f;                      // Минимальная ширина поля ввода
    float paddingX          = 10.0f;                       // Внутренний горизонтальный отступ поля ввода

    ImU32 colIcon           = 0; // Иконка в обычном состоянии
    ImU32 colIconHover      = 0; // Иконка при наведении
    ImU32 colIconActive     = 0;  // Иконка в режиме редактирования
    ImU32 colBtnBg          = 0;    // Фон кнопки карандаша
    ImU32 colBtnBorder      = 0;    // Рамка кнопки карандаша
    ImU32 colInputBg        = 0;    // Фон поля ввода
    ImU32 colInputBorder    = 0;  // Рамка поля ввода (акцент)
};

// Стиль кнопки панели инструментов (ToolButton)
struct ToolButtonStyle {
    float size              = 28.0f;                       // Базовый размер стороны квадратной кнопки
    float cornerRadius      = 6.0f;                        // Радиус скругления рамки
    float borderSize        = 1.0f;                        // Толщина рамки
    float iconScale         = 0.55f;                       // Масштаб иконки внутри кнопки

    ImFont* font            = nullptr;                     // Шрифт для текстовых глифов
    float fontSize          = 16.0f;

    ImU32 colBg             = 0;    // Фон кнопки
    ImU32 colBgHover        = 0;    // Фон при наведении
    ImU32 colBgActive       = 0;    // Фон при клике
    ImU32 colBorder         = 0;    // Тонкая рамка
    ImU32 colBorderHover    = 0;   // Рамка при наведении
    ImU32 colIcon           = 0;  // Цвет иконки / текста
    ImU32 colIconHover      = 0;  // Цвет иконки при наведении

    ImU32 colBgSelected     = 0;  // Фон выбранного тумблера
    ImU32 colBorderSelected = 0;  // Рамка выбранного тумблера
};

// Стиль таблицы (Table)
struct TableStyle {
    // 1. Шапка таблицы (Header)
    ImU32 colHeaderBg          = 0;   // Фон шапки
    ImU32 colHeaderHovered     = 0;   // Подсветка кликабельного заголовка при сортировке
    ImU32 colHeaderActive      = 0;   // Заголовок при нажатии
    ImU32 colHeaderText        = 0;// Вторичный текст заголовков
    float headerHeight         = 40.0f;                       // Базовая высота шапки
    float headerFontSize       = 18.0f;                       // Базовый размер шрифта шапки (18pt Medium)
    float cellFontSize         = 20.0f;                       // Базовый размер шрифта ячеек (20pt Medium)
    float subFontSize          = 18.0f;                       // Базовый размер подзаголовков (18pt Regular)

    ImU32 colBodyBg            = 0;   // Подложка тела таблицы и пустого состояния

    // 2. Строки данных (Rows)
    ImU32 colRowBg             = 0;   // Основной фон строки
    ImU32 colRowBgAlt          = 0;   // Чередующаяся зебра
    ImU32 colRowHovered        = 0;   // Подсветка при наведении курсора
    ImU32 colRowSelected       = 0;  // Выделенная строка
    float rowHeight            = 64.0f;                       // Базовая высота строки

    // 3. Сетка и границы ячеек (Borders & Dividers)
    ImU32 colBorderOuter       = 0;   // Внешняя рамка
    ImU32 colBorderInner       = 0;   // Внутренние тонкие разделители
    float cellPaddingX         = 12.0f;                       // Горизонтальный паддинг ячейки
    float cellPaddingY         = 8.0f;                        // Вертикальный паддинг ячейки

    // 4. Пустое состояние (Empty state)
    ImU32 colEmptyIcon         = 0;
    ImU32 colEmptyText         = 0;
};

// Стиль внутристраничной панели фильтрации и поиска (FilterBar)
struct FilterBarStyle {
    float height            = 34.0f;                       // Базовая высота панели фильтрации
    float itemSpacing       = 8.0f;                        // Шаг между однотипными кнопками в группе
    float groupSpacing      = 14.0f;                       // Шаг между разными смысловыми группами (действия -> фильтры)
    float labelSpacing      = 6.0f;                        // Шаг от подписи до её контрола (Label -> Combo/Input)
    float stretchGap        = 16.0f;                       // Зазор между растягивающимся поиском и правой группой
    float minSearchWidth    = 180.0f;                      // Минимальная ширина поля поиска
    ImU32 colLabel          = 0;// Цвет подписи (muted)
};

// Стиль полосы прокрутки (Scrollbar)
struct ScrollbarStyle {
    float size              = 8.0f;                        // Аккуратная толщина 8px (вместо дефолтных 16px)
    float cornerRadius      = 4.0f;                        // Полное скругление бегунка («пилюля» / капсула)
    float minGrabSize       = 20.0f;                       // Минимальная длина бегунка при больших списках

    ImU32 colBg             = 0;        // Прозрачная дорожка (не режет фон таблицы/контента)
    ImU32 colBgHovered      = 0;   // Едва заметная подложка при наведении
    ImU32 colGrab           = 0;  // Бегунок
    ImU32 colGrabHovered    = 0; // Ярче при наведении мыши
    ImU32 colGrabActive     = 0; // Бегунок при перетаскивании
};

// Стиль вертикального списка элементов (List / ListView)
struct ListStyle {
    // Типографика (строго чётные кегли)
    ImFont* headerFont         = nullptr; // Шрифт заголовка секции списка
    float headerFontSize       = 16.0f;   // Заголовок секции (16pt Bold)
    ImFont* itemFont           = nullptr; // Шрифт названия элемента
    float itemFontSize         = 18.0f;   // Шрифт элемента (18pt Medium)
    ImFont* subFont            = nullptr; // Шрифт подписи / деталей
    float subFontSize          = 14.0f;   // Шрифт деталей (14pt Regular)

    // Геометрия
    float itemHeight           = 38.0f;   // Базовая высота строки
    float itemSpacing          = 2.0f;    // Зазор между строками
    float cornerRadius         = 6.0f;    // Скругление строки при наведении/выделении
    float paddingX             = 10.0f;   // Внутренний горизонтальный отступ
    float activeBarWidth       = 3.0f;    // Ширина акцентной вертикальной полоски выделения
    float iconSize             = 14.0f;   // Размер векторной иконки / точки статуса

    // Палитра
    ImU32 colBg                = 0;         // Фон списка (прозрачный)
    ImU32 colItemBg            = 0;         // Фон обычного элемента в покое
    ImU32 colItemHoverBg       = 0;    // Фон элемента при наведении
    ImU32 colItemSelectedBg    = 0;    // Фон выбранного элемента
    ImU32 colActiveBar         = 0;  // Акцентная полоска

    ImU32 colHeader            = 0; // Текст заголовка списка
    ImU32 colText              = 0; // Текст обычного элемента
    ImU32 colTextSelected      = 0; // Текст выбранного элемента
    ImU32 colTextMuted         = 0; // Вторичный текст / подпись
    ImU32 colBorder            = 0;    // Рамка списка/разделителя
};

// Стиль области основного контента (ContentArea)
// Рабочая область — холст приложения: своего фона не рисует, под ней кадр,
// очищенный цветом UiTheme::colWindowBg
struct ContentAreaStyle {
    float paddingX             = 16.0f;                        // Внутренний горизонтальный отступ
    float paddingY             = 10.0f;                        // Внутренний вертикальный отступ
    bool enableScrollX         = false;                        // Горизонтальная прокрутка
    bool enableScrollY         = false;                        // Вертикальная прокрутка
};

// Стиль интерактивного тега-чипа (Tag)
struct TagStyle {
    ImFont* font               = nullptr; // Шрифт тега
    float fontSize             = 16.0f;   // Базовый размер шрифта (16pt Medium)

    float paddingX             = 6.0f;    // Внутренний горизонтальный отступ
    float paddingY             = 1.5f;    // Внутренний вертикальный отступ
    float spacingX             = 5.0f;    // Зазор между тегами по горизонтали
    float spacingY             = 2.5f;    // Зазор между строками тегов при переносе
    float cornerRadius         = 4.0f;    // Радиус скругления плашки
    float borderSize           = 1.0f;    // Толщина рамки

    ImU32 colBg                = 0;    // Фон в покое
    ImU32 colBgHover           = 0;    // Фон при наведении
    ImU32 colBgActive          = 0;    // Фон при нажатии
    ImU32 colBorder            = 0;    // Рамка в покое
    ImU32 colBorderHover       = 0;  // Рамка при наведении
    ImU32 colPrefix            = 0; // Цвет решетки '#'
    ImU32 colPrefixHover       = 0;
    ImU32 colText              = 0; // Цвет текста тега
    ImU32 colTextHover         = 0;
};

// Стиль интерактивной строки-карточки списка (ItemRow)
struct ItemRowStyle {
    // Типографика (строго чётные кегли)
    ImFont* titleFont          = nullptr; // Шрифт названия
    float titleFontSize        = 20.0f;   // 20pt Bold
    ImFont* symbolFont         = nullptr; // Шрифт символа
    float symbolFontSize       = 18.0f;   // 18pt Bold
    ImFont* unitFont           = nullptr; // Шрифт единицы
    float unitFontSize         = 18.0f;   // 18pt Medium
    ImFont* descriptionFont    = nullptr; // Шрифт описания
    float descriptionFontSize  = 16.0f;   // 16pt Regular
    ImFont* formulaFont        = nullptr; // Шрифт формулы
    float formulaFontSize      = 16.0f;   // 16pt Medium
    float badgeFontSize        = 14.0f;   // 14pt Medium

    // Геометрия и внутренние отступы (поджатые, без лишнего воздуха)
    float paddingX             = 10.0f;   // Горизонтальный отступ внутри карточки
    float paddingYTop = 4.0f;             // Верхний отступ внутри карточки
    float paddingYBottom = 4.0f;          // Нижний отступ внутри карточки
    float spacingTitleDesc     = 1.0f;    // Отступ между заголовком и описанием
    float spacingDescTags      = 2.0f;    // Отступ между описанием и тегами
    float itemSpacingY = 8.0f;            // Зазор между карточками в списке
    float cornerRadius         = 6.0f;    // Скругление карточки
    float borderSize           = 1.0f;    // Толщина рамки карточки

    // Кнопка действия справа
    float actionBtnWidth       = 34.0f;   // Ширина кнопки действия
    float actionBtnHeight      = 34.0f;   // Высота кнопки действия
    float actionSpacing        = 5.0f;    // Зазор между кнопками действий

    // Палитра
    ImU32 colBg                = 0;    // Фон карточки
    ImU32 colBgHover           = 0;    // Фон при наведении
    ImU32 colBorder            = 0;    // Рамка карточки
    ImU32 colBorderHover       = 0;   // Рамка при наведении
    ImU32 colTextDescription   = 0; // Цвет текста описания
    ImU32 colTextFormula       = 0; // Цвет формулы
};

// Цветовая триада семантического бейджа: тинт-подложка, рамка, текст варианта
struct BadgeVariantColors {
    ImU32 colBg;       // Цвет фона бейджа
    ImU32 colBorder;   // Цвет рамки бейджа
    ImU32 colText;     // Цвет текста (высококонтрастный, легко читаемый)
};

// Стиль семантического бейджа-шильдика (Badge)
struct BadgeStyle {
    ImFont* font               = nullptr; // Шрифт бейджа (по умолчанию fontMedium)
    float fontSize             = 14.0f;   // Базовый размер шрифта (14pt Medium)

    float paddingX             = 6.0f;    // Внутренний горизонтальный отступ
    float paddingY             = 1.5f;    // Внутренний вертикальный отступ
    float cornerRadius         = 4.0f;    // Скругление плашки
    float borderSize           = 1.0f;    // Толщина рамки

    // Цвета по вариантам UiVariant (Default=0, Primary=1, Secondary=2, Success=3, Warning=4, Danger=5, Info=6)
    BadgeVariantColors colors[7] = {};

    const BadgeVariantColors& GetColors(UiVariant variant) const {
        size_t idx = static_cast<size_t>(variant);
        if (idx < sizeof(colors) / sizeof(colors[0])) {
            return colors[idx];
        }
        return colors[0];
    }
};

// ============================================================================
// 4. Главный класс дизайн-системы (UiTheme)
// ============================================================================
class UiTheme {
public:
    static UiTheme& Get();

    UiTheme();

    // Выключенное состояние (DisabledScope): всё внутри выключенной области
    // приглушается этой прозрачностью. Компоненты рисуют свои цвета через
    // ImGui::GetColorU32(color) — так они приглушаются вместе с цветами ImGui;
    // цвета, переданные в PushStyleColor, ImGui приглушает сам (повторно не
    // оборачивать)
    static constexpr float kDisabledAlpha = 0.45f;

    // 1. Масштабирование
    static constexpr float kMinScale = 0.75f;
    static constexpr float kMaxScale = 2.5f;
    float scale = 1.0f;
    float GetScale() const { return scale; }
    void SetScale(float s);
    float Scale(float baselinePx) const { return baselinePx * scale; }

    // 2. Опорные размеры каркаса (Shell Layout)
    float headerHeight     = 80.0f;
    float topBarHeight     = 46.0f;
    float sidebarWidth     = 230.0f;

    float HeaderHeight() const { return Scale(header.height); }
    float StatusBarHeight() const { return Scale(statusBar.height); }
    float ToolbarHeight() const { return Scale(toolbar.height); }
    float TopBarHeight() const { return ToolbarHeight(); }
    float SidebarWidth() const { return Scale(sidebarWidth); }

    // 3. Стандартная сетка отступов (Grid Spacing Tokens)
    float spacingSmall     = 4.0f;
    float spacingMedium    = 8.0f;
    float spacingLarge     = 16.0f;

    float SpacingSmall() const { return Scale(spacingSmall); }
    float SpacingMedium() const { return Scale(spacingMedium); }
    float SpacingLarge() const { return Scale(spacingLarge); }

    // Вспомогательные делегаты для совместимости
    float CornerRadius() const { return Scale(card.cornerRadius); }
    float IndicatorWidth() const { return Scale(indicator.width); }
    float IndicatorHeight() const { return Scale(indicator.height); }
    float TabHeight() const { return Scale(tab.height); }
    void SetTabHeight(float h) { tab.height = h; }
    float CardRowSpacing() const { return Scale(card.rowSpacing); }
    void SetCardRowSpacing(float s) { card.rowSpacing = s; }
    float FieldLabelSpacing() const { return Scale(input.labelSpacing); }
    void SetFieldLabelSpacing(float s) { input.labelSpacing = s; }

    // 4. Общие шрифты интерфейса (динамический FreeType атлас)
    ImFont* fontRegular      = nullptr; // Roboto-Regular: базовый текст, ячейки, метки
    ImFont* fontMedium       = nullptr; // Roboto-Medium: кнопки, вкладки, значения полей
    ImFont* fontBold         = nullptr; // Roboto-Bold: заголовки карточек, пульт, табло

    // Алиасы для обратной совместимости
    ImFont* defaultFont      = nullptr; // -> fontRegular
    ImFont* smallFont        = nullptr; // -> fontRegular
    ImFont* buttonFont       = nullptr; // -> fontMedium
    ImFont* projectTitleFont = nullptr; // -> fontBold
    ImFont* displayBigFont   = nullptr; // -> fontBold

    // Вспомогательные методы динамического стека шрифтов ImGui 1.92+
    void PushFont(ImFont* font, float basePt) const {
        ImFont* f = font ? font : (fontRegular ? fontRegular : defaultFont);
        ImGui::PushFont(f, Scale(basePt));
    }

    void PushFont(float basePt) const {
        PushFont(nullptr, basePt);
    }

    void PopFont() const {
        ImGui::PopFont();
    }

    // 5. Тема оформления: палитра (семантика цветов) текущего режима
    ThemeMode mode = ThemeMode::Dark;
    ThemePalette palette = ThemePalette::Dark();

    // Смена темы: палитра -> стили компонентов -> ImGuiStyle
    void SetMode(ThemeMode m);

    // 6. Стили конкретных компонентов
    TabBarStyle tab;
    CardStyle card;
    InputFieldStyle input;
    ComboStyle combo;
    ToggleStyle toggle;
    SidebarStyle sidebar;
    IndicatorStyle indicator;
    ValueDisplayStyle valueDisplay;
    HeaderStyle header;
    StatusBarStyle statusBar;
    DeviceStatusStyle deviceStatus;
    CarouselStyle carousel;
    ContextMenuStyle contextMenu;
    PresetGridStyle presetGrid;
    SidePanelStyle sidePanel;
    ToolbarStyle toolbar;
    EditableLabelStyle editableLabel;
    ToolButtonStyle toolButton;
    TableStyle table;
    FilterBarStyle filterBar;
    ScrollbarStyle scrollbar;
    ListStyle list;
    ContentAreaStyle contentArea;
    ChartStyle chart;
    ItemRowStyle itemRow;
    TagStyle tag;
    BadgeStyle badge;

    // Получение масштабированных метрик контрола под размер UiSize
    ControlMetrics GetMetrics(UiSize size) const;

    // Получение семантического стиля варианта (цвета фона, текста, рамок)
    const SemanticStyle& GetVariantStyle(UiVariant variant) const;

    // Загрузка шрифтов интерфейса и компонентов
    void LoadFonts();

    // Обновление масштабирования (DPI scale)
    void UpdateScaling(float s) { SetScale(s); }

    // Применение базовых параметров и цветов к ImGuiStyle
    void ApplyToImGui();

    // Системный масштаб интерфейса (DPI монитора); 1.0, если не определить
    static float SystemScale();

private:
    // Цвета всех стилей компонентов и семантических вариантов — из палитры
    void ApplyPalette();

    SemanticStyle m_variants[7] = {};
};
