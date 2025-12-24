#pragma once

#include <QtCore/qlogging.h>
#include <format>

#define AIKARI_UI_SUBPROJ_NAME "Aikari-UI-Settings"

#ifndef AIKARI_UI_MODULE_NAME
#define AIKARI_UI_MODULE_NAME "Unknown"
#endif

#define Q_LOG_ACTION_GET_LOGGER_MID_DESC \
    "[" AIKARI_UI_SUBPROJ_NAME "] <" AIKARI_UI_MODULE_NAME ">"

// clang-format off
#define Q_LOG_DEBUG qDebug() << Q_LOG_ACTION_GET_LOGGER_MID_DESC << "\033[0;30m\033[47m|DEBUG|\033[0m"
#define Q_LOG_INFO qInfo() << Q_LOG_ACTION_GET_LOGGER_MID_DESC << "\033[0;37m\033[46m|INFO|\033[0m"
#define Q_LOG_WARN qWarning() << Q_LOG_ACTION_GET_LOGGER_MID_DESC << "\033[0;30m\033[43m|WARN|\033[0m"
#define Q_LOG_ERROR qWarning() << Q_LOG_ACTION_GET_LOGGER_MID_DESC << "\033[0;37m\033[41m|ERROR|\033[0m"
#define Q_LOG_CRITICAL qCritical() << Q_LOG_ACTION_GET_LOGGER_MID_DESC << "\033[0;37m\033[45m|CRITICAL|\033[0m"
// clang-format on
