#include "Cartesian.h"

namespace mms {

Cartesian::Cartesian() {
    m_x = Meters(0.0);
    m_y = Meters(0.0);
}

Cartesian::Cartesian(const Distance& x, const Distance& y) {
    m_x = x;
    m_y = y;
}

Cartesian::Cartesian(const Coordinate& coordinate) {
    m_x = coordinate.getX();
    m_y = coordinate.getY();
}

Cartesian Cartesian::operator+(const Coordinate& coordinate) const {
    return Cartesian(getX() + coordinate.getX(), getY() + coordinate.getY());
}

Cartesian Cartesian::operator-(const Coordinate& coordinate) const {
    return Cartesian(getX() - coordinate.getX(), getY() - coordinate.getY());
}

Cartesian Cartesian::operator*(double factor) const {
    return Cartesian(getX() * factor, getY() * factor);
}

Cartesian Cartesian::operator/(double factor) const {
    return Cartesian(getX() / factor, getY() / factor);
}

void Cartesian::operator+=(const Coordinate& coordinate) {
    m_x += coordinate.getX();
    m_y += coordinate.getY();
}

} // namespace mms
