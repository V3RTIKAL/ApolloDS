# ApolloDS provenance

## Base

ApolloDS is based on Nonary/Vibepollo release `1.18.4-stable.4`, commit `e3ebca75ec06f975699bf38f46012b96db9c3e44`.

## Dual-screen host donor

- Repository: `https://github.com/JoeCorrell/sunshine-ds`
- Commit: `25be3eba19c6667939d3202ef9ce188d16db8baa`
- Subject: `feat: add Sunshine DS dual-display streaming`
- Material semantics selected for adaptation:
  - `MaxVideoStreams=2` capability
  - `video/1/0` RTSP SETUP and PLAY
  - indexed second-video SDP attributes
  - independent second UDP/RTP video transport
  - independent capture and encoder state
  - indexed IDR/reference-frame invalidation
  - display-indexed absolute input
  - secondary failure isolation

The donor's historical virtual-display and capture architecture is not transplanted. Those behaviors are implemented through Vibepollo's current VDISPLAY, display-helper, capture, encoder, session, validation and recovery facilities.

Existing copyright and license notices remain authoritative for base and donor-derived source.
