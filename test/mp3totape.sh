#!/bin/bash
if ! [ -e "$1" ]
then
echo ==================================================================
echo 'Please specify an .mp3 file to convert it to 11025kHz signed 8 bit'
echo 'mono raw - with "-r 44100", the resulting file can be used with'
echo 'vanilla pce-ibmpc'
echo ==================================================================
fi
set -euo pipefail
INFILE="$1"
OUTFILE="${1,,}"
OUTFILE=${2:-${OUTFILE%.mp3}.raw}
sox "$INFILE" -r 11025 -c 1 -b 8 -s "$OUTFILE"
# # similar tools like mplayer can be used instead of sox
# mplayer \
#    -ao pcm:fast:waveheader:file="${OUTFILE}" \
#    -format s8 \
#    -vo null -vc null \
#    "${INFILE}"
