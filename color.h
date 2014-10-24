#ifndef COLOR_H
#define COLOR_H

class Color {
public:
    // Constructors
    Color();
    Color(uint8_t r, uint8_t g, uint8_t b);
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    // Destructor
    ~Color();
    
    // Methods
    uint32_t ToValue();
    
    // Static helpers
    static Color FromValue(const uint32_t value);
    static Color BlendAlpha(const Color colors[]);
    
private:
    uint8_t r, g, b, a;
};

// Helper functions

#endif // COLOR_H
