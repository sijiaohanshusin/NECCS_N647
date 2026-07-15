/**
 * @file app_ui_bridge.h
 * @brief Single facade between the application layer and the TouchGFX UI.
 *
 * Architecture rule: UI code (Model/Presenter/View) may only include this
 * header to reach the application layer. It aggregates the snapshot getters
 * and request entry points the UI is allowed to use; anything not exported
 * here is off-limits to the UI.
 *
 * Data flow:
 *   - App threads publish snapshots (acoustic, PCMD, camera/display, touch,
 *     power, media); Model::tick polls them through these headers.
 *   - UI intent (mode changes, media commands, overlay/visibility) is pushed
 *     back through the Request/Set functions re-exported below.
 */
#ifndef APP_UI_BRIDGE_H
#define APP_UI_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_acoustic_service.h"
#include "app_beam_record.h"
#include "app_bringup_thread.h"
#include "app_camera.h"
#include "app_camera_display.h"
#include "app_gimbal.h"
#include "app_media.h"
#include "app_npu.h"
#include "app_pcmd_capture.h"
#include "app_power.h"
#include "TOUCH/app_touch.h"

#ifdef __cplusplus
}
#endif

#endif /* APP_UI_BRIDGE_H */
