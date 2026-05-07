#!/bin/bash
# Generate Enigma Virtual Box .evb project from deploy directory contents
DST_DIR="d:/3.OpenFOAM/0.OpenFOAM/OpenFOAMGUI/packaging/deploy"
EVB_FILE="$DST_DIR/OpenFOAMGUI.evb"
BOXED_EXE="$DST_DIR/OpenFOAMGUI_boxed.exe"

T="    "  # 4-space tab

{
  echo '<?xml encoding="utf-16"?>'
  echo '<>'
  echo "$T<InputFile>$DST_DIR/OpenFOAMGUI.exe</InputFile>"
  echo "$T<OutputFile>$BOXED_EXE</OutputFile>"
  echo "$T<Files>"
  echo "$T${T}<Enabled>True</Enabled>"
  echo "$T${T}<DeleteExtractedOnExit>True</DeleteExtractedOnExit>"
  echo "$T${T}<CompressFiles>True</CompressFiles>"
  echo "$T${T}<Files>"
  echo "$T${T}${T}<File>"
  echo "$T${T}${T}${T}<Type>3</Type>"
  echo "$T${T}${T}${T}<Name>%DEFAULT FOLDER%</Name>"
  echo "$T${T}${T}${T}<Action>0</Action>"
  echo "$T${T}${T}${T}<OverwriteDateTime>False</OverwriteDateTime>"
  echo "$T${T}${T}${T}<OverwriteAttributes>False</OverwriteAttributes>"
  echo "$T${T}${T}${T}<HideFromDialogs>0</HideFromDialogs>"
  echo "$T${T}${T}${T}<Files>"

  # Add all files from deploy directory
  find "$DST_DIR" -type f -not -name "OpenFOAMGUI_boxed.exe" -not -name "OpenFOAMGUI.evb" -not -name "*.evb" | while IFS= read -r f; do
    rel="${f#$DST_DIR/}"
    echo "$T${T}${T}${T}${T}<File>"
    echo "$T${T}${T}${T}${T}${T}<Type>2</Type>"
    echo "$T${T}${T}${T}${T}${T}<Name>$rel</Name>"
    echo "$T${T}${T}${T}${T}${T}<File>$f</File>"
    echo "$T${T}${T}${T}${T}${T}<ActiveX>False</ActiveX>"
    echo "$T${T}${T}${T}${T}${T}<ActiveXInstall>False</ActiveXInstall>"
    echo "$T${T}${T}${T}${T}${T}<Action>0</Action>"
    echo "$T${T}${T}${T}${T}${T}<OverwriteDateTime>False</OverwriteDateTime>"
    echo "$T${T}${T}${T}${T}${T}<OverwriteAttributes>False</OverwriteAttributes>"
    echo "$T${T}${T}${T}${T}${T}<PassCommandLine>False</PassCommandLine>"
    echo "$T${T}${T}${T}${T}${T}<HideFromDialogs>0</HideFromDialogs>"
    echo "$T${T}${T}${T}${T}</File>"
  done

  echo "$T${T}${T}${T}</Files>"
  echo "$T${T}${T}</File>"
  echo "$T${T}</Files>"
  echo "$T</Files>"
  echo '</>'
} > "$EVB_FILE"

echo "EVB generated: $EVB_FILE"
wc -l "$EVB_FILE"
