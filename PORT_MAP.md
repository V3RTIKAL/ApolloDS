# ApolloDS host port map

## Frozen revisions

- Base: Nonary/Vibepollo `1.18.4-stable.4` at `e3ebca75ec06f975699bf38f46012b96db9c3e44`
- Canonical donor: JoeCorrell/sunshine-ds `25be3eba19c6667939d3202ef9ce188d16db8baa`
- Rule: preserve Vibepollo architecture; port only the donor's one-session/two-video semantics.

## Integration map

| Milestone | Donor behavior | Vibepollo target | Action |
|---|---|---|---|
| H1 | `MaxVideoStreams=2` when stream 1 is genuinely available | `src/nvhttp.cpp` server-info generation | ADAPT; retain current encoder/VDD discovery |
| H1 | Strict indexed RTSP target parsing (`video/0/0`, `video/1/0`) | `src/rtsp.cpp`, new pure helpers in `src/stream_protocol.{h,cpp}` | REIMPLEMENT AGAINST CURRENT API |
| H1 | Explicit `x-ml-video[1].enable` and indexed viewport/FPS/bitrate | `rtsp_stream::cmd_announce`, `launch_session_t` | ADAPT; stream 1 remains optional |
| H1 | Aggregate primary-first bitrate budget | protocol helper plus `cmd_announce` | COPY SMALL LOGIC with overflow validation |
| H1/H2 | Reserve UDP offset 12 across SETUP-to-ANNOUNCE | `launch_session_t`, broadcast socket lifecycle, `src/stream.h` | REIMPLEMENT AGAINST CURRENT API |
| H2 | Independent UDP/RTP sender, sequence/FEC/IV/IDR state | `src/stream.cpp`, `src/globals.h` | ADAPT existing sender with indexed state; do not copy whole sender |
| H2 | Secondary-only failure and indexed termination | `stream::session_t`, control server | ADAPT; never call whole-session stop for stream 1 failure |
| H3 | Independent capture/encoder publishing to video queue 2 | `src/video.cpp`, `src/video.h`, `src/stream.cpp` | REIMPLEMENT AGAINST CURRENT API using explicit output name |
| H3 | Separate shutdown/HDR/touch-port/packet channels | `safe::mail` declarations and capture context | ADAPT minimal per-stream channels |
| H4 | Indexed IDR and reference-frame invalidation | `src/stream_protocol.{h,cpp}`, control handlers | COPY SMALL LOGIC with exact-length validation |
| H4 | Display-indexed absolute mouse/touch | `src/input.cpp`, `src/input.h` | ADAPT current validation and permission-aware input path |
| H4 | Relative mouse remains global | current relative mouse path | ALREADY EXISTS; do not bind it to a display |
| H5 | Lease one secondary display for stream 1 | `src/platform/windows/virtual_display*`, display helper and `display_device` | REIMPLEMENT AGAINST CURRENT API |
| H5 | Stable, distinct per-client secondary identity | VDISPLAY stable UUID facilities | ADAPT; never reuse primary VDD identity |
| H5 | Secondary-only cleanup | identity-scoped VDISPLAY removal/recovery | REIMPLEMENT; never use global remove-all/revert |
| Packaging | UDP offset 12 exposure | `src/upnp.cpp`, firewall/installer definitions | ADAPT only where current packaging requires it |

## Explicitly not ported

- Donor `src/dual_display.cpp` driver/topology implementation: Vibepollo already owns VDD backends, recovery, topology and display-helper integration.
- Donor capture/encoder files wholesale: they predate current Vibepollo WGC, HDR, adapter, pacing and shutdown behavior.
- Any process-global output override for stream 1.
- Two sessions, two audio streams, composite framebuffers, or a second host process.

## Test-first slices

1. Indexed target parsing and bitrate budgeting in `stream_protocol`.
2. Indexed IDR/reference invalidation parsing.
3. Second-port reservation lifetime and distinct SETUP ports.
4. Independent sender lifecycle and secondary-only failure.
5. Secondary display identity/lease ownership.
6. Explicit-output second capture without changing primary runtime output.
7. Per-display absolute input and independent touch cancellation.
8. Single-stream compatibility and full-session teardown.

Hardware-only checks remain: real encoder concurrency, Windows extended topology/VDD cleanup, and independent display capture.
