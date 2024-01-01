;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; NSIS installer script for ArrowDL ;
; (http://nsis.sourceforge.net)     ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;---------------
; FIND WINDOWS 
;---------------     
Var STR_HAYSTACK
Var STR_NEEDLE
Var STR_CONTAINS_VAR_1
Var STR_CONTAINS_VAR_2
Var STR_CONTAINS_VAR_3
Var STR_CONTAINS_VAR_4
Var STR_RETURN_VAR

;-------------------------------------------------------------------------
; Create the shared function.
;${un} : Trick to share functions between installer and uninstaller
!macro MYMACRO un
;-------------------------------------------------------------------------

    Function ${un}quitIfRunning
        ;Use 'Microsoft Spy++' to find ArrowDL's window
        ;Visual Studio > Tools > Microsoft Spy++ 
        ; * class name = "Qt5QWindowIcon" 
        ; * title name = "0/0 - ArrowDL v1.7.1"
        Push  0   # the starting offset of the push below
        Push  "- ArrowDL v"   # the known part of the title name
        Call  ${un}enhancedFindWindow
        Pop  $0   # will contain the full wcn
        Pop  $1   # will contain the window's handle
                  # both will contain "failed", if no matching below name was found
        StrCmp $0 "failed" gotoLabelNotRunning
            MessageBox MB_OK|MB_ICONEXCLAMATION "ArrowDL is running. Please close it first" /SD IDOK
            Abort
        gotoLabelNotRunning:
    FunctionEnd

    Function ${un}enhancedFindWindow
        ; input, save variables
        Exch  $0   # part of the title name to search for
        Exch
        Exch  $1   # starting offset
        Push  $2   # length of $0
        Push  $3   # window handle
        Push  $4   # title name
        Push  $5   # temp

        ; set up the variables
        StrCpy  $4  0
        StrLen  $2  $0
         
        ; loop to search for open windows
search_loop:
        ; FindWindow  $3  "SunAwtFrame"  ""  0  $3
        ; FindWindow  $3  ""  ""  0  $3
        FindWindow  $3  "Qt5QWindowIcon"  ""  0  $3
        IntCmp  $3  0  search_failed
            IsWindow  $3  0  search_loop
                 System::Call 'user32.dll::GetWindowText(i r3, t .r4, i ${NSIS_MAX_STRLEN}) i .n'
                 StrCmp  $4  ""  search_loop
                 Push $4
                 Push $0
                 Call ${un}strContains
                 Pop $5
            ;MessageBox MB_OK "TITLE: $4 ++ $5"  ; uncomment for debug
            StrCmp $5 "" search_loop  search_end
        
        ; no matching class-name found, return "failed"
search_failed:
        StrCpy  $3  "failed"
        StrCpy  $4  "failed"
        
        ; search ended, output and restore variables
search_end:
        StrCpy  $1  $3
        StrCpy  $0  $4
        Pop  $5
        Pop  $4
        Pop  $3
        Pop  $2
        Exch  $1
        Exch
        Exch  $0
    FunctionEnd     
     
    ; search for a string
    Function ${un}strContains
        Exch $STR_NEEDLE
        Exch 1
        Exch $STR_HAYSTACK
        ; Uncomment to debug
        ;MessageBox MB_OK 'STR_NEEDLE = $STR_NEEDLE STR_HAYSTACK = $STR_HAYSTACK '
        StrCpy $STR_RETURN_VAR ""
        StrCpy $STR_CONTAINS_VAR_1 -1
        StrLen $STR_CONTAINS_VAR_2 $STR_NEEDLE
        StrLen $STR_CONTAINS_VAR_4 $STR_HAYSTACK
loop:
        IntOp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_1 + 1
        StrCpy $STR_CONTAINS_VAR_3 $STR_HAYSTACK $STR_CONTAINS_VAR_2 $STR_CONTAINS_VAR_1
        StrCmp $STR_CONTAINS_VAR_3 $STR_NEEDLE found
        StrCmp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_4 done
        Goto loop
found:
        StrCpy $STR_RETURN_VAR $STR_NEEDLE
        Goto done
done:
        Pop $STR_NEEDLE ;Prevent "invalid opcode" errors and keep the
        Exch $STR_RETURN_VAR  
    FunctionEnd

!macroend

; Insert function as an installer and uninstaller function.
!insertmacro MYMACRO "."
!insertmacro MYMACRO "un."
