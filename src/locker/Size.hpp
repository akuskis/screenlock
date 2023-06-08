#pragma once

template <typename T>
class Size
{
public:
    Size()
      : width_{}
      , height_{}
    {
    }

    Size(T width, T height)
      : width_(width)
      , height_(height)
    {
    }

    void setWidth(T width)
    {
        width_ = width;
    }

    void setHeight(T height)
    {
        height_ = height;
    }

    [[nodiscard]] T getWidth() const
    {
        return width_;
    }

    [[nodiscard]] T getHeight() const
    {
        return height_;
    }

private:
    T width_, height_;
};
