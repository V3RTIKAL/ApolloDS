/**
 * @file globals.h
 * @brief Declarations for globally accessible variables and functions.
 */
#pragma once

// local includes
#include "entry_handler.h"
#include "thread_pool.h"

/**
 * @brief A thread pool for processing tasks.
 */
extern thread_pool_util::ThreadPool task_pool;

/**
 * @brief A boolean flag to indicate whether the cursor should be displayed.
 */
extern bool display_cursor;

#ifdef _WIN32
  // Declare global singleton used for NVIDIA control panel modifications
  #include "platform/windows/nvprefs/nvprefs_interface.h"

/**
 * @brief A global singleton used for NVIDIA control panel modifications.
 */
extern nvprefs::nvprefs_interface nvprefs_instance;
#endif

/**
 * @brief Handles process-wide communication.
 */
namespace mail {
#define MAIL(x) \
  constexpr auto x = std::string_view { \
    #x \
  }

  /**
   * @brief A process-wide communication mechanism.
   */
  extern safe::mail_t man;

  // Global mail
  MAIL(shutdown);
  MAIL(broadcast_shutdown);
  MAIL(video_packets);
  MAIL(video_packets2);  ///< Independently queued packets for video stream 1.
  MAIL(audio_packets);
  MAIL(switch_display);

  // Local mail
  MAIL(touch_port);
  MAIL(touch_port2);
  MAIL(idr);
  MAIL(idr2);
  MAIL(invalidate_ref_frames);
  MAIL(invalidate_ref_frames2);
  MAIL(gamepad_feedback);
  MAIL(hdr);
  MAIL(hdr2);
  MAIL(dynamic_bitrate);  // Runtime encoder bitrate change (kbps), posted from the HTTP /bitrate handler
  MAIL(dynamic_bitrate2);  // Secondary encoder bitrate updates remain isolated from the primary encoder.
#undef MAIL

}  // namespace mail
