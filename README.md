# archi_lab6

# 0a: <br />
* add to the main the signal handlers for ignore 3 commenads (as descripted in the exam).<br />
* add a handler fun which test which kind of sig is it, and if it one of the 3 above- prind an ignored message.<br />

--------------------------------------------------------------------------
# 0b:<br />
options of process control signals: <br />
* & - after the process name whill run the process in background.<br />
* ctrl+z - send the process from the foreground to the background (= pause the process)<br />
* jobs   - print all the current processes<br />
* fg  - send the last suspended process to foreground.<br />
* fg%id  - do the same as above to a specific process with the exact id number<br />
* bg  - continue a suspended process in background. Or by id or the last suspended one.<br />
* kill - kill jobs by their id.<br />

--------------------------------------------------------------------------
# 0c:<br />
the only thing that needed to be done in this section:  change the executeTwoPipes to multiExecutePipes.<br />
change:<br />
in child2 before execute check if you're the last process (by check line->next->next!=0). <br/>
* if true ->there are more commands after you -> call multiExecutePipes (line->next)  <br/>
* else -> you're the last process -> go to execute func.<br/>
--------------------------------------------------------------------------
# 1a:<br />
implement some of the job's linked list. the funcs are: <br/>
* initialize_job(char* cmd) <br/>
  malloc for cmd and tmodes for the new node. <br/>
  all the other fields initial with the defaults. <br/>
* find_job_by_index(job * job_list, int idx)<br/>
  while that run on the list, if job_list.idx==idx ->return job_list.idx<br/>
  else ->next.<br/>
  if end of while ->return null.<br/>
* void update_job_list(job ** job_list, int remove_done_jobs)<br/>
  run on the list, do a non-blocking waitpid (<=> WNOHANG) <br/>
  for eanch job in the list: print it and remove from list.<br/>
--------------------------------------------------------------------------
# 1b:<br />
add the jobs command: <br/>
* in job_control -implement the update_job_list func (print and refresh the list). <br/>
* in main.c - strcmp to the string- <br>
  if =="jobs -> update_job_list and then free the mallocs. <br/>
  else- do all the regular calculation.<br/>
  --------------------------------------------------------------------------
# 1c:<br />

  



