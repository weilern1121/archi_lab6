# archi_lab6

0a: <br />
** add to the main the signal handlers for ignore 3 commenads (as descripted in the exam).<br />
** add a handler fun which test which kind of sig is it, and if it one of the 3 above- prind an ignored message.<br />

--------------------------------------------------------------------------
0b:<br />
options of process control signals: <br />
**  &      - after the process name whill run the process in background.<br />
**  ctrl+z - send the process from the foreground to the background (= pause the process)<br />
**  jobs   - print all the current processes<br />
**     fg  - send the last suspended process to foreground.<br />
**  fg%id  - do the same as above to a specific process with the exact id number<br />
**     bg  - continue a suspended process in background. Or by id or the last suspended one.<br />
**    kill - kill jobs by their id.<br />

--------------------------------------------------------------------------
0c:<br />
the only thing that needed to be done in this section:  change the executeTwoPipes to multiExecutePipes.<br />
change:<br />
in child2 before execute check if you're the last process (by check line->next->next!=0). <br/>
** if true ->there are more commands after you -> call multiExecutePipes (line->next)  <br/>
* else -> you're the last process -> go to execute func.<br/>
--------------------------------------------------------------------------
1a:<br />




