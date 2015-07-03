#include "TileGraphic.h"

#include "Colors.h"
#include "GraphicUtilities.h"
#include "Param.h"
#include "State.h"

namespace sim {

TileGraphic::TileGraphic(const Tile* tile) : m_tile(tile), m_color(COLOR_STRINGS.at(P()->tileBaseColor())),
        m_text(""), m_foggy(true) {
}

bool TileGraphic::wallDeclared(Direction direction) const {
    return m_declaredWalls.find(direction) != m_declaredWalls.end();
}

void TileGraphic::setColor(const GLfloat* color) {
    m_color = color;
    updateColor();
}

void TileGraphic::setText(const std::string& text) {
    m_text = text;
}

void TileGraphic::setFogginess(bool foggy) {
    m_foggy = foggy;
    updateFog();
}

void TileGraphic::declareWall(Direction direction, bool isWall) {
    m_declaredWalls[direction] = isWall;
    updateWall(direction);
}

void TileGraphic::undeclareWall(Direction direction) {
    m_declaredWalls.erase(direction);
    updateWall(direction);
}

void TileGraphic::draw() const {

    // Draw the base of the tile
    GraphicUtilities::drawTileGraphicBase(m_tile->getX(), m_tile->getY(), m_tile->getFullPolygon(),
        S()->tileColorsVisible() ? m_color : COLOR_STRINGS.at(P()->tileBaseColor()));

    // Draw each of the walls of the tile
    for (Direction direction : DIRECTIONS) {

        // Get the wall color and alpha
        std::pair<const GLfloat*, GLfloat> colorAndAlpha = deduceWallColorAndAlpha(direction);

        // Draw the polygon
        GraphicUtilities::drawTileGraphicWall(m_tile->getX(), m_tile->getY(), direction,
            m_tile->getWallPolygon(direction), colorAndAlpha.first, colorAndAlpha.second);
    }

    // Draw the corners of the tile
    std::vector<Polygon> cornerPolygons = m_tile->getCornerPolygons();
    for (int cornerNumber = 0; cornerNumber < cornerPolygons.size(); cornerNumber += 1) {
        GraphicUtilities::drawTileGraphicCorner(m_tile->getX(), m_tile->getY(), cornerNumber,
            cornerPolygons.at(cornerNumber), COLOR_STRINGS.at(P()->tileCornerColor()));
    }

    // Draw the fog
    GraphicUtilities::drawTileGraphicFog(m_tile->getX(), m_tile->getY(), m_tile->getFullPolygon(),
        COLOR_STRINGS.at(P()->tileFogColor()), m_foggy && S()->tileFogVisible() ? P()->tileFogAlpha() : 0.0);

    // TODO TODO TODO TODO : fix this
    // Draw the tile text, always padded to at least 3 characters (for distances)
    /*
    if (S()->tileTextVisible()) {
        glColor3fv(COLOR_STRINGS.at(P()->tileTextColor()));
        GraphicUtilities::drawText(
            Cartesian(Meters(m_tile->getX() * (P()->wallWidth() + P()->wallLength()) + P()->wallWidth()),
                      Meters(m_tile->getY() * (P()->wallWidth() + P()->wallLength()) + P()->wallWidth())),
            Meters(P()->wallLength()), Meters(P()->wallLength() / 3.0),
            std::string("   ").substr(0, (0 > 3 - (int) m_text.size() ? 0 : 3 - m_text.size())) + m_text);
    }
    */

    // TODO: Draw the shortest path graphic, which is usefule because this is
    // the metric by which mice that don't reach the center are ranked

}

void TileGraphic::updateColor() const {
    GraphicUtilities::updateTileGraphicBaseColor(m_tile->getX(), m_tile->getY(),
        S()->tileColorsVisible() ? m_color : COLOR_STRINGS.at(P()->tileBaseColor()));
}

void TileGraphic::updateWalls() const {
    for (Direction direction : DIRECTIONS) {
        updateWall(direction);
    }
}

void TileGraphic::updateFog() const {
    GraphicUtilities::updateTileGraphicFog(m_tile->getX(), m_tile->getY(),
        m_foggy && S()->tileFogVisible() ? P()->tileFogAlpha() : 0.0);
}

void TileGraphic::updateWall(Direction direction) const {
    std::pair<const GLfloat*, GLfloat> colorAndAlpha = deduceWallColorAndAlpha(direction);
    GraphicUtilities::updateTileGraphicWallColor(m_tile->getX(), m_tile->getY(), direction,
        colorAndAlpha.first, colorAndAlpha.second);
}

std::pair<const GLfloat*, GLfloat> TileGraphic::deduceWallColorAndAlpha(Direction direction) const {

    // Declare the wall color and alpha
    const GLfloat* wallColor;
    GLfloat wallAlpha;

    // Either draw the true walls of the tile ...
    if (S()->wallTruthVisible()) {
        wallColor = COLOR_STRINGS.at(P()->tileWallColor());
        wallAlpha = m_tile->isWall(direction) ? 1.0 : 0.0;
    }

    // ... or the algorithm's (un)declared walls
    else {

        // (un)declared walls have an alpha value of 1.0 (except for one case)
        wallAlpha = 1.0;

        // If the wall was declared, use the wall color and tile base color ...
        if (m_declaredWalls.find(direction) != m_declaredWalls.end()) {
            if (m_declaredWalls.at(direction)) {
                if (m_tile->isWall(direction)) {
                    wallColor = COLOR_STRINGS.at(P()->tileWallColor());
                }
                else {
                    wallColor = COLOR_STRINGS.at(P()->tileIncorrectlyDeclaredWallColor());
                }
            }
            else {
                if (m_tile->isWall(direction)) {
                    wallColor = COLOR_STRINGS.at(P()->tileIncorrectlyDeclaredNoWallColor());
                }
                else {
                    wallAlpha = 0.0;
                }
            }
        }

        // ... otherwise, use the undeclared walls colors
        else {
            if (m_tile->isWall(direction)) {
                wallColor = COLOR_STRINGS.at(P()->tileUndeclaredWallColor());
            }
            else {
                wallColor = COLOR_STRINGS.at(P()->tileUndeclaredNoWallColor());
            }
        }
    }

    return std::make_pair(wallColor, wallAlpha);
}

} // namespace sim
