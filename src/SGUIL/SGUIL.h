/*
    SGUIL - Simple GUI Layer version 0.1.0 for SDL2 >=2.0.5
*/
#pragma once
#define SGUIL_VERSION_STR "0.1.2"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include "SDL.h"
#include <string>
#include <utility>
#include <vector>
#define GUI_TEXT_ALIGN_CENTER   0x0002
#define GUI_TEXT_OUTLINE        0x0004
#define GUI_TEXT_SHADOW         0x0008
#define GUI_TEXT_HIGHLIGHT      0x0010
#define GUI_TEXT_ALIGN_RIGHT    0x0020
#define GUI_RGBA_DEFAULT            0xFFFFFFFF
#define GUI_RGBA_OUTLINE_DEFAULT    0x000000FF
#define GUI_WINDOW_CALLBACK_NONE    NULL
typedef unsigned char byte_t;
typedef uint32_t rgba_t;
typedef int optionID_t;
extern SDL_Renderer *Gui_SDL_Renderer;
extern SDL_Texture *Gui_ThemeTexture;
bool Gui_Init(SDL_Renderer *, const char *);
struct BitFont
// sheets should be 32w x 4h characters in dimensions
{
    BitFont() : sheet(NULL), outlineSheet(NULL), charW(0), charH(0) {isValid = false;}
    BitFont(const char *, const char *, unsigned int, unsigned int);
    ~BitFont();

    bool isValid;

    SDL_Texture *sheet;
    SDL_Texture *outlineSheet;
    unsigned int charW;
    unsigned int charH;
};

enum class enumAlignment
{
    left, right, center, justified
};

struct TextFormat
{
    TextFormat()
        : TextFormat(0, GUI_RGBA_DEFAULT, GUI_RGBA_OUTLINE_DEFAULT) {}

    TextFormat(rgba_t rgba, rgba_t rgbaOutline)
        : TextFormat(0, rgba, rgbaOutline) {}

    TextFormat(unsigned int flags, rgba_t rgba, rgba_t rgbaOutline)
        : rgba(rgba), rgbaOutline(rgbaOutline)
    {
        rgbaHighlight = 0x8080FFFF;

        highlight = flags & GUI_TEXT_HIGHLIGHT;
        outline = flags & GUI_TEXT_OUTLINE;
        shadow = flags & GUI_TEXT_SHADOW;
        sizeMult = 1.0;
        lineSpacing = 1.0;
        alignment = enumAlignment::left;
        wrapLen = 0;

        if(flags & GUI_TEXT_ALIGN_RIGHT)
        {
            alignment = enumAlignment::right;
        } else if(flags & GUI_TEXT_ALIGN_CENTER)
        {
            alignment = enumAlignment::center;
        }
    }

    rgba_t rgba;
    rgba_t rgbaOutline;
    rgba_t rgbaHighlight;

    bool highlight;
    bool outline;
    bool shadow;

    float sizeMult;
    float lineSpacing;
    enumAlignment alignment;
    unsigned int wrapLen;
};

// extern std::map<std::string, BitFont> Gui_MyBitFonts;

enum enumGuiEventType
{
event_invalid = 0,
mouse_guievent = 0x1000,
    mouse_clicked,
    mouse_released,
    mouse_dragged,
    mouse_hovered_onto,
    mouse_hovered_off,
    mouse_moved,
key_guievent = 0x2000,
    key_pressed,
    key_released,
textinput_guievent = 0x2FFF,
joy_guievent = 0x4000,
    joybutton_pressed,
    joybutton_released
};

enum enumMouseButtonType
{
    mouse_button_left, mouse_button_middle, mouse_button_right
};

struct GuiPoint
{
    GuiPoint() : x(0), y(0) {}
    GuiPoint(int x, int y) : x(x), y(y) {}
    int x;
    int y;
};

typedef GuiPoint GuiRelativePoint;
typedef GuiPoint GuiVirtualPoint;

struct GuiMouseEvent
{
    GuiMouseEvent() : x(0), y(0), button(0) {}
    GuiMouseEvent(int x, int y) : x(x), y(y), button(0) {}
    GuiMouseEvent(int x, int y, Uint8 button) : x(x), y(y), button(button) {}

    int x;
    int y;
    Uint8 button;
};
/*
struct GuiMouseButtonEvent
{
    GuiMouseButtonEvent() {x = 0; y = 0; button = SDL_BUTTON_LEFT;}
    GuiMouseButtonEvent(int x, int y, Uint8 button) : x(x), y(y), button(button) {}

    int x;
    int y;
    Uint8 button;
};
*/
struct GuiKeyEvent
{
    GuiKeyEvent() {key = SDLK_UNKNOWN;}
    GuiKeyEvent(SDL_Keycode kc) : key(kc) {}

    SDL_Keycode key;
};

struct GuiTextInputEvent
{
    GuiTextInputEvent() : text() {}
    GuiTextInputEvent(std::string s) : text(s) {}

    std::string text;
};

struct GuiEvent
{
    GuiEvent() : type(event_invalid), eventUnion(nullptr) {}
    GuiEvent(enumGuiEventType type, int x, int y, Uint8 button)
        : type(type), eventUnion(nullptr), mouseEvent(x, y, button)
    {
        switch(type)
        {
            case mouse_clicked:
                mouseClickedEvent = &mouseEvent;
                break;
            case mouse_released:
                mouseReleasedEvent = &mouseEvent;
                break;
            case mouse_dragged:
                mouseDraggedEvent = &mouseEvent;
                break;
            case mouse_hovered_onto:
                mouseHoveredOntoEvent = &mouseEvent;
                break;
            case mouse_hovered_off:
                mouseHoveredOffEvent = &mouseEvent;
                break;
            case mouse_moved:
                mouseMovedEvent = &mouseEvent;
                break;
            default:
                break;
        }
    }

    GuiEvent(enumGuiEventType type, SDL_Keycode kc)
        : type(type), keyEvent(kc)
    {
        switch(type)
        {
            case key_pressed:
                keyPressedEvent = &keyEvent;
                break;
            case key_released:
                keyReleasedEvent = &keyEvent;
                break;
            default:
                eventUnion = nullptr;
                break;
        }
    }

    GuiEvent(enumGuiEventType type, std::string s)
        : type(type), typingEvent(s)
    {
        textInputEvent = &typingEvent;
    }

    enumGuiEventType type;

    union
    {
        GuiMouseEvent *mouseClickedEvent;
        GuiMouseEvent *mouseReleasedEvent;
        GuiMouseEvent *mouseDraggedEvent;
        GuiMouseEvent *mouseHoveredOntoEvent;
        GuiMouseEvent *mouseHoveredOffEvent;
        GuiMouseEvent *mouseMovedEvent;

        GuiKeyEvent *keyPressedEvent;
        GuiKeyEvent *keyReleasedEvent;

        GuiTextInputEvent *textInputEvent;

        void *eventUnion;
    };

private:
    GuiMouseEvent mouseEvent;
    GuiKeyEvent keyEvent;
    GuiTextInputEvent typingEvent;
};

class GuiWindow;

class GuiElement
// base class for all gui objects
{
public:
    GuiElement() : containingWindow(nullptr), relativeDestRect({ 0, 0, 0, 0 }) {}
    virtual ~GuiElement() {};

    virtual void draw() = 0;
    void prepareRenderTarget(bool);
    void setWindow(GuiWindow& w)
    {
        containingWindow = &w;
    }
    GuiWindow *getWindow()
    {
        return containingWindow;
    }

protected:
    GuiWindow *containingWindow;
    // relativeDestRect's x, y fields are relative to container's canvas texture
    SDL_Rect relativeDestRect;
};

class GuiText : public GuiElement
// simple text box with (probably) unchanging text, can be used for explanations and tooltips
{
public:
    GuiText(std::string, BitFont&, SDL_Rect&);
    ~GuiText() {}

    void draw();

private:
    std::string text;
    std::vector<std::pair<int, int>> textPositionalValues;
    TextFormat fmt;
    BitFont& font;

    bool updatePositionalValues;
};

class GuiInteractable : public GuiElement
// base class for all gui elements than can be interacted with
{
public:
    GuiInteractable() :
        enabled(false),
        canHoldKeyboardFocus(false),
        hasDefaultKeyboardFocus(false),
        ID(0),
        selected(false),
        hasKeyboardFocus(false),
        displayTexture(nullptr),
        updateDisplayStringPVs(false) {}

    // canInteractAt: checks if the given mouse X and Y are inside the element's rectangle,
    // and if the element is enabled. if so, either perform some action or let the user do so
    virtual bool canInteractAt(int, int);
    virtual void mouseClicked(int, int, Uint8) {}
    virtual void mouseDragged(int, int, Uint8) {}
    virtual void mouseReleased(int, int, Uint8) {}
    virtual void keyPressed(SDL_Keycode) {}
    virtual void textInput(std::string) {}

    virtual void handleEvent(GuiEvent&);

    bool enabled;
    bool canHoldKeyboardFocus;
    bool hasDefaultKeyboardFocus;
    int ID;

    // these are set by either the user or the parent window if there is one
    bool selected;
    bool hasKeyboardFocus;

protected:
    std::string displayString;
    std::vector<std::pair<int, int>> displayStringPositionalValues;
    SDL_Texture *displayTexture;
    bool updateDisplayStringPVs;
};

class BindableVariable;

class VariableObserver
{
public:
    virtual ~VariableObserver() {}
    virtual void call(BindableVariable *) = 0;
};

class StaticVariableObserver : public VariableObserver
{
public:
    StaticVariableObserver(std::function<void(BindableVariable *)> func) : func(func) {}
    virtual void call(BindableVariable *bv) override
    {
        func(bv);
    }

protected:
    std::function<void(BindableVariable *)> func;
};

template<typename T>
class IndirectObjVariableObserver : public VariableObserver
{
public:
    IndirectObjVariableObserver(T& obj, void (*proxy)(T&, BindableVariable *)) : obj(obj), proxy(proxy) {}
    virtual void call(BindableVariable *bv) override
    {
        proxy(obj, bv);
    }

protected:
    T& obj;
    void (*proxy)(T&, BindableVariable *);
};

class MemberVariableObserver : public VariableObserver
{
public:
    MemberVariableObserver(std::function<void(BindableVariable *)> membFunc) : membFunc(membFunc) {}
    virtual void call(BindableVariable *bv) override
    {
        membFunc(bv);
    }

protected:
    std::function<void(BindableVariable *)> membFunc;
};

/* maybe useful for something in the future? possibility for parameter packs*/
// class TemplatedObserver : public VariableObserver

class BindableVariable
{
public:
    BindableVariable(const std::string& name) : name_(name) {}
    virtual ~BindableVariable() {}

    std::string name() const { return name_; }
    virtual void set(const std::string& val) = 0;
    virtual std::string get() const = 0;

    void addObserver(std::unique_ptr<VariableObserver>& ob)
    {
        observers.push_back(std::move(ob));
    }

protected:
    void valueChanged()
    {
        for(auto& ob : observers)
        {
            ob->call(this);
        }
    }

private:
    const std::string name_;
    std::vector<std::unique_ptr<VariableObserver>> observers;
};

class BindableString : public BindableVariable
{
public:
    BindableString(const std::string& name) : BindableVariable(name) {}
    virtual void set(const std::string& val) override
    {
        if(val != value)
        {
            value = val;
            valueChanged();
        }
    }

    virtual std::string get() const override { return value; }

private:
    std::string value;
};

class BindableInt : public BindableVariable
{
public:
    // min is inclusive, max is exclusive
    BindableInt(const std::string& name, int64_t min, int64_t max)
        : BindableVariable(name), value(0), min(min), max(max) {}

    virtual void set(const std::string& val) override { setInt(stoll(val)); }
    virtual std::string get() const override { return std::to_string(getInt()); }


    void setInt(int64_t&& val)
    {
        if(min <= val && val < max)
        {
            if(val != value)
            {
                value = val;
                valueChanged();
            }
        }
    }

    int64_t getInt() const { return value; }

    std::pair<int64_t, int64_t> getRange() const { return { min, max }; }

protected:
    int64_t value;
    const int64_t min;
    const int64_t max;
};

class BindableEnumeration : public BindableInt
{
public:
    BindableEnumeration(const std::string& name, std::vector<std::string>&& values)
        : BindableInt(name, 0, values.size()), values(std::move(values)) {}

    virtual void set(const std::string& val) override
    {
        auto it = std::find(values.begin(), values.end(), val);
        if(it != values.end())
        {
            auto newval = it - values.begin();
            if(newval != value)
            {
                value = newval;
                valueChanged();
            }
        }
    };

    virtual std::string get() const override
    {
        return displayNameForValue(value);
    }

    std::string displayNameForValue(size_t val) const
    {
        assert(val < values.size());
        return values[val];
    };

private:
    std::vector<std::string> values;
};

class BindableFloat : public BindableVariable
{
public:
    BindableFloat(const std::string& name, long double min, long double max)
        : BindableVariable(name), value(0.0), min(min), max(max) {}

    virtual void set(const std::string& val) override { setFloat(stold(val)); }
    virtual std::string get() const override { return std::to_string(getFloat()); }


    void setFloat(long double&& val)
    {
        if(min <= val && val < max)
        {
            if(val != value)
            {
                value = val;
                valueChanged();
            }
        }
    }

    long double getFloat() const { return value; }

    std::pair<long double, long double> getRange() const { return { min, max }; }

protected:
    long double value;
    const long double min;
    const long double max;
};

class BindableVariables
{
public:
    void add(std::unique_ptr<BindableVariable> var)
    {
        assert(!find(var->name()));
        vars.emplace_back(std::move(var));
    };

    BindableVariable* find(const std::string& name)
    {
        if(vars.size() == 0)
        {
            return NULL;
        }

        auto it = std::find_if(vars.begin(), vars.end(), [&name](const auto& p){ return p->name() == name; });
        if(it != vars.end())
        {
            return it->get();
        }

        return NULL;
    };

private:
    std::vector<std::unique_ptr<BindableVariable>> vars;
};

enum enumOptionAccess
{
    no_access           = 0,
    use_callback        = 1 << 0,
    random_access_copy  = 1 << 1,
    random_access       = 1 << 2
};

inline enumOptionAccess operator | (enumOptionAccess a, enumOptionAccess b)
{ return static_cast<enumOptionAccess>(static_cast<int>(a) | static_cast<int>(b)); }

inline enumOptionAccess operator |= (enumOptionAccess a, enumOptionAccess b)
{ return a = a | b; }

class GuiOptionInteractable : public GuiInteractable
{
public:
    GuiOptionInteractable() : var(nullptr) {}

protected:
    BindableVariable *var;
    //std::function<int(optionID_t, T)> valueUpdateCallback;
    //optionID_t optionID;

    //std::vector<T> valueOptions;
    //int choice;
    //T value;

    //enumOptionAccess accessType;
    //T *accessPtr;
};

// template<typename T>
class GuiTextField : public GuiOptionInteractable
// text box which the user can send input to
{
public:
    GuiTextField(int, BindableString *, BitFont&, SDL_Rect);
    GuiTextField(int, BindableString *, std::string, BitFont&, SDL_Rect);
    // GuiTextField(int, optionID_t, std::string, BitFont&, SDL_Rect, std::function<int(optionID_t, std::string)>, std::string *, bool);
    // GuiTextField(const GuiTextField&);
    ~GuiTextField();

    void setTextFormat(TextFormat&);

    void draw();

    void handleEvent(GuiEvent&);

    void mouseClicked(int, int, Uint8);
    void mouseDragged(int, int, Uint8);
    void mouseReleased(int, int, Uint8);
    void keyPressed(SDL_Keycode);
    void textInput(std::string);

    unsigned int getPositionUnderMouse(int, int);

    unsigned int shiftCursor(int);
    bool textInsert(std::string);
    void textDelete(); // delete selection
    void textDelete(unsigned int, unsigned int);
    std::string textCut();
    std::string textCopy();

protected:
    std::vector<std::pair<int, int>> textPositionalValues;
    std::string value;
    TextFormat fmt;
    BitFont& font;
    uint64_t lastEventTime;
    bool typing;
    unsigned int cursor;
    unsigned int scrollPosX;
    unsigned int scrollPosY;

    unsigned int selectionStart;
    unsigned int selectionEnd;

    bool updatePositionalValues;
    bool enableNewline;
    bool horizontalScroll;
    bool verticalScroll;
};

class GuiButton : public GuiInteractable
// clickable, causes a direct action to take place, can also be scrolled through with arrow keys or similar
{
public:
    GuiButton(int ID, SDL_Rect relativeDestRect, std::string displayString, BitFont& font);
    // GuiButton(const GuiButton&);
    ~GuiButton();

    void draw();

    void mouseClicked(int, int, Uint8);
    void mouseDragged(int, int, Uint8);
    void mouseReleased(int, int, Uint8);
    void keyPressed(SDL_Keycode);

protected:
    BitFont &font;
};

struct CoreState;

class GuiWindow
// essentially a container for GuiElement with relative positioning, possibly with -oX controls like in a WM
{
public:
    GuiWindow(CoreState *origin, std::string, BitFont *, std::function<void(GuiInteractable&, GuiEvent&)>, SDL_Rect&);
    ~GuiWindow();

    void draw();

    // void addText(GuiText&);
    // void addTextField(GuiTextField&);
    // void addButton(GuiButton&);

    std::string& getTitle() {return title;}

    void addElement(GuiElement *);
    void addControlElement(GuiInteractable *);

    void handleSDLEvent(SDL_Event&, GuiPoint);
    GuiInteractable *getControlElementAt(int, int);

    void moveRect(int, int);

    void mouseMoved(int, int);
    void mouseClicked(int, int, Uint8);
    void mouseDragged(int, int, Uint8);
    void mouseReleased(int, int, Uint8);
    void keyPressed(SDL_Keycode);
    void textInput(std::string);

    CoreState *origin;
    SDL_Texture *canvas;
    SDL_Rect destRect;

protected:
    std::vector<GuiElement *> elements;
    std::vector<GuiInteractable *> controlList;
    int keyboardFocus;
    int controlSelection;
    bool selectingByMouse;

    std::function<void(GuiInteractable&, GuiEvent&)> interactionEventCallback;

    rgba_t rgbaBackground;
    rgba_t rgbaTitleBar;

    std::string title;
    std::vector<std::pair<int, int>> titlePositionalValues;
    BitFont titleFont;
    int titleBarHeight;

    bool showBackground;
    bool showTitleBar;
    bool updateTitlePositionalValues;
    bool moveable;

    bool useExtWindowX;
    bool useExtWindowY;
    bool moving;
    int initialX;
    int initialY;
    int moveBeginX;
    int moveBeginY;
};

class GuiScreen : public GuiWindow
{
public:
    GuiScreen(CoreState *origin, std::string title, std::function<void(GuiInteractable&, GuiEvent&)> interactionEventCallback, SDL_Rect& destRect)
        : GuiWindow(origin, title, NULL, interactionEventCallback, destRect)
    {
        name = title;

        moveable = false;
        showBackground = false;
        showTitleBar = false;

        useExtWindowX = true;
        useExtWindowY = true;
    }

    void setParent(std::string p) { parent = p; }
    void addChild(std::string c) { children.push_back(c); }

protected:
    std::string name;
    std::string parent;
    std::vector<std::string> children;
};

/*
class GuiPanel : public GuiWindow
{

};
*/

inline byte_t rgba_R(rgba_t rgba)
{
    return (rgba & 0xFF000000) / 0x1000000;
}

inline byte_t rgba_G(rgba_t rgba)
{
    return (rgba & 0x00FF0000) / 0x0010000;
}

inline byte_t rgba_B(rgba_t rgba)
{
    return (rgba & 0x0000FF00) / 0x0000100;
}

inline byte_t rgba_A(rgba_t rgba)
{
    return (rgba & 0x000000FF);
}

inline void GuiElement::prepareRenderTarget(bool isFinalCopy) {
    if(this->containingWindow)
    {
        SDL_SetRenderTarget(Gui_SDL_Renderer, this->containingWindow->canvas);
    } else
    {
        SDL_SetRenderTarget(Gui_SDL_Renderer, NULL);
    }
}

inline void Gui_SetTextureRGBA(SDL_Texture *tex, rgba_t rgba)
{
    SDL_SetTextureColorMod(tex, rgba_R(rgba), rgba_G(rgba), rgba_B(rgba));
    SDL_SetTextureAlphaMod(tex, rgba_A(rgba));
}

void Gui_DrawBorder(SDL_Rect&, int, rgba_t);

void Gui_GenerateTextPositionalValues(std::string&, TextFormat *, BitFont&, SDL_Rect&, std::vector<std::pair<int, int>>&, bool, bool);
void Gui_GenerateTextPositionalValuesPartial(std::string&, unsigned int, unsigned int, TextFormat *, BitFont&, SDL_Rect&, std::vector<std::pair<int, int>>&, bool, bool);

void Gui_DrawText_PV(std::string, TextFormat *, BitFont&, std::vector<std::pair<int, int>>&, unsigned int, unsigned int);
void Gui_DrawTextPartial_PV(std::string, unsigned int, unsigned int, TextFormat *, BitFont&, std::vector<std::pair<int, int>>&, unsigned int, unsigned int);
void Gui_DrawText(std::string, TextFormat *, BitFont&, SDL_Rect&);
void Gui_DrawTextPartial(std::string, unsigned int, unsigned int, TextFormat *, BitFont&, SDL_Rect&);