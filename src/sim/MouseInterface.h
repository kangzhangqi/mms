#pragma once

#include <QMap>
#include <QObject>
#include <QPair>

#include "DynamicMouseAlgorithmOptions.h"
#include "InterfaceType.h"
#include "MazeView.h"
#include "Mouse.h"
#include "Param.h"

#define ENSURE_DISCRETE_INTERFACE ensureDiscreteInterface(__func__);
#define ENSURE_CONTINUOUS_INTERFACE ensureContinuousInterface(__func__);
#define ENSURE_ALLOW_OMNISCIENCE ensureAllowOmniscience(__func__);
#define ENSURE_NOT_TILE_EDGE_MOVEMENTS ensureNotTileEdgeMovements(__func__);
#define ENSURE_USE_TILE_EDGE_MOVEMENTS ensureUseTileEdgeMovements(__func__);
#define ENSURE_INSIDE_ORIGIN ensureInsideOrigin(__func__);
#define ENSURE_OUTSIDE_ORIGIN ensureOutsideOrigin(__func__);

namespace mms {

class MouseInterface : public QObject {

    Q_OBJECT

public:

    MouseInterface(
        const Maze* maze,
        Mouse* mouse,
        MazeView* view);

    // Called when the algo process writes to stdout
    void handleStandardOutput(QString output);

    // Called when the algo process could not start
    void emitMouseAlgoCannotStart(QString string);

    // Execute a request, return a response
    QString dispatch(const QString& command);

    // Request that the mouse algorithm exit
    void requestStop();

    // A user pressed an input button in the UI
    void inputButtonWasPressed(int button);

    // Parameters set by the algorithm
    InterfaceType getInterfaceType(bool canFinalize) const;
    DynamicMouseAlgorithmOptions getDynamicOptions() const;

signals:

    // Emit sanitized algorithm output
    void algoOutput(QString output);

    // An algorithm acknowledged an input button
    void inputButtonWasAcknowledged(int button);

    // The algorithm could not be started
    void mouseAlgoCannotStart(QString errorString);

private:

    // *********************** START PUBLIC INTERFACE ******************** //

    // ----- Any interface methods ----- //

    // Config-related functions
    char getStartedDirection();
    void setStartingDirection(char direction);
    void setWheelSpeedFraction(double wheelSpeedFraction);

    // Misc functions
    double getRandom();
    int millis(); // # of milliseconds of sim time (adjusted based on sim speed) that have passed
    void delay(int milliseconds); // # of milliseconds of sim time (adjusted based on sim speed)

    // Tile color
    void setTileColor(int x, int y, char color);
    void clearTileColor(int x, int y);
    void clearAllTileColor();

    // Tile text
    void setTileText(int x, int y, const QString& text);
    void clearTileText(int x, int y);
    void clearAllTileText();

    // Tile walls
    void declareWall(int x, int y, char direction, bool wallExists);
    void undeclareWall(int x, int y, char direction);

    // Tile fog
    void setTileFogginess(int x, int y, bool foggy);

    // Tile distance, where a negative distance corresponds to inf distance
    void declareTileDistance(int x, int y, int distance);
    void undeclareTileDistance(int x, int y);

    // Reset position of the mouse
    void resetPosition();

    // Input buttons
    bool inputButtonPressed(int inputButton);
    void acknowledgeInputButtonPressed(int inputButton);

    // ----- Continuous interface methods ----- //

    // Get the magnitude of the max speed of any one wheel in rpm
    double getWheelMaxSpeed(const QString& name);

    // Set the speed of any one wheel
    void setWheelSpeed(const QString& name, double rpm);

    // Get the number of encoder ticks per revolution for a wheel
    double getWheelEncoderTicksPerRevolution(const QString& name);

    // Read the encoder for a particular wheel
    int readWheelEncoder(const QString& name);

    // Reset the encoder for a particular wheel to zero, but only if the encoder type is relative
    void resetWheelEncoder(const QString& name);

    // Returns a value in [0.0, 1.0]
    double readSensor(const QString& name);

    // Returns deg/s of rotation
    double readGyro();

    // ----- Any discrete interface methods ----- //

    bool wallFront();
    bool wallRight();
    bool wallLeft();

    // ----- Basic discrete interface methods ----- //

    void moveForward();
    void moveForward(int count);

    void turnLeft();
    void turnRight();

    void turnAroundLeft();
    void turnAroundRight();

    // ----- Special discrete interface methods ----- //

    void originMoveForwardToEdge();
    void originTurnLeftInPlace();
    void originTurnRightInPlace();

    void moveForwardToEdge();
    void moveForwardToEdge(int count);

    void turnLeftToEdge();
    void turnRightToEdge();

    void turnAroundLeftToEdge();
    void turnAroundRightToEdge();

    void diagonalLeftLeft(int count);
    void diagonalLeftRight(int count);
    void diagonalRightLeft(int count);
    void diagonalRightRight(int count);

    // ----- Omniscience methods ----- //

    int currentXTile();
    int currentYTile();
    char currentDirection();

    double currentXPosMeters();
    double currentYPosMeters();
    double currentRotationDegrees();

    // ************************ END PUBLIC INTERFACE ********************* //

    // Pointers to various simulator objects
    const Maze* m_maze;
    Mouse* m_mouse;
    MazeView* m_view;

    // The interface type (DISCRETE or CONTINUOUS)
    InterfaceType m_interfaceType;
    mutable bool m_interfaceTypeFinalized;

    // The runtime algorithm options
    DynamicMouseAlgorithmOptions m_dynamicOptions;

    // Whether or a stop was requested
    bool m_stopRequested;

    // Whether or not the input buttons are pressed/acknowleged
    QMap<int, bool> m_inputButtonsPressed;

    // Whether or not the mouse has moved out the origin
    bool m_inOrigin;

    // A constant used to ensure that the mouse
    // doesn't travel too fast in DISCRETE mode
    double m_wheelSpeedFraction;

    // Cache of tiles, for making clearAll methods faster
    std::set<QPair<int, int>> m_tilesWithColor;
    std::set<QPair<int, int>> m_tilesWithText;

    // Helper methods for checking particular conditions and failing hard
    void ensureDiscreteInterface(const QString& callingFunction) const;
    void ensureContinuousInterface(const QString& callingFunction) const;
    void ensureAllowOmniscience(const QString& callingFunction) const;
    void ensureNotTileEdgeMovements(const QString& callingFunction) const;
    void ensureUseTileEdgeMovements(const QString& callingFunction) const;
    void ensureInsideOrigin(const QString& callingFunction) const;
    void ensureOutsideOrigin(const QString& callingFunction) const;

    // Implementation methods:
    // Any functionality that is executed as part of another MouseInterface
    // method should have an Impl method, and the Impl method should be called.
    // As an important note, if any methods can cause a crash, it must handle
    // the crash appropriately (see moveForwardImpl() for an example).
    void setTileColorImpl(int x, int y, char color);
    void clearTileColorImpl(int x, int y);
    void setTileTextImpl(int x, int y, const QString& text);
    void clearTileTextImpl(int x, int y);
    void declareWallImpl(
        QPair<QPair<int, int>, Direction> wall, bool wallExists, bool declareBothWallHalves);
    void undeclareWallImpl(
        QPair<QPair<int, int>, Direction> wall, bool declareBothWallHalves);
    bool wallFrontImpl(bool declareWallOnRead, bool declareBothWallHalves);
    bool wallLeftImpl(bool declareWallOnRead, bool declareBothWallHalves);
    bool wallRightImpl(bool declareWallOnRead, bool declareBothWallHalves);
    void moveForwardImpl(bool originMoveForwardToEdge = false);
    void turnLeftImpl();
    void turnRightImpl();
    void turnAroundLeftImpl();
    void turnAroundRightImpl();
    void turnToEdgeImpl(bool turnLeft);
    void turnAroundToEdgeImpl(bool turnLeft);

    // Helper methods for wall retrieval and declaration
    bool isWall(QPair<QPair<int, int>, Direction> wall, bool declareWallOnRead, bool declareBothWallHalves);
    bool hasOpposingWall(QPair<QPair<int, int>, Direction> wall) const;
    QPair<QPair<int, int>, Direction> getOpposingWall(
        QPair<QPair<int, int>, Direction> wall) const;

    // Some helper abstractions for mouse movements
    void moveForwardTo(const Cartesian& destinationTranslation, const Radians& destinationRotation);
    void arcTo(const Cartesian& destinationTranslation, const Radians& destinationRotation,
        const Meters& radius, double extraWheelSpeedFraction);
    void turnTo(const Cartesian& destinationTranslation, const Radians& destinationRotation);

    // Returns the angle with from "from" to "to", with values in [-180, 180) degrees
    Radians getRotationDelta(const Radians& from, const Radians& to) const;

    // Returns the center of a given tile
    Cartesian getCenterOfTile(int x, int y) const;

    // Returns the location of where the mouse should stop if it crashes
    QPair<Cartesian, Degrees> getCrashLocation(
        QPair<int, int> currentTile, Direction destinationDirection);

    // TODO: MACK - rename to Impl
    void doDiagonal(int count, bool startLeft, bool endLeft);

};

} // namespace mms
