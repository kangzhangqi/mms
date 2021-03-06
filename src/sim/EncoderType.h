#pragma once

#include <QDebug>
#include <QMap>
#include <QString>

#ifdef _WIN32
    #include "Windows.h"
#endif

#include "ContainerUtilities.h"

namespace mms {

enum class EncoderType {
    ABSOLUTE,
    RELATIVE,
};

const QMap<EncoderType, QString>& ENCODER_TYPE_TO_STRING();
const QMap<QString, EncoderType>& STRING_TO_ENCODER_TYPE();

inline QDebug operator<<(QDebug stream, EncoderType encoderType) {
    stream.noquote() << ENCODER_TYPE_TO_STRING().value(encoderType);
    return stream;
}

} // namespace mms
