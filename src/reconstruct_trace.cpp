
#include "ParaverTrace.h"
#include "ParaverTraceThread.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceApplication.h"
#include "spectral-api.h"

#include <iostream>
#include <fstream>
#include <vector>

std::vector<Period_t *> Periods;

unsigned long long *CurrentIterTime;
int                *CurrentPeriod;

using namespace std;

namespace libparaver {

class Process : public ParaverTrace
{
  private:
    int num_tasks;
    ofstream traceout;

  public:
    Process (string prvFile, bool multievents, string tracename, int num_detected_periods, Period_t **detected_periods);
    ~Process ();

    void processState (struct state_t &s);
    void processMultiEvent (struct multievent_t &e);
    void processEvent (struct singleevent_t &e);
    void processCommunication (struct comm_t &c);
    void processCommunicator (string &c);
    void processComment (string &c);
};


Process::Process (string prvFile, bool multievents, string tracename, int num_detected_periods, Period_t **detected_periods) : ParaverTrace (prvFile, multievents)
{
        vector<ParaverTraceApplication *> va = get_applications();
        if (va.size() != 1)
        {
                cerr << "ERROR Cannot parse traces with more than one application" << endl;
                exit (-1);
        }
        num_tasks = va[0]->get_tasks().size();

        CurrentIterTime = (unsigned long long *)malloc(num_tasks * sizeof(unsigned long long));
        CurrentPeriod   = (int *)malloc(num_tasks * sizeof(int));

	traceout.open (tracename.c_str());
	if (!traceout.is_open())
	{
		cout << "Unable to create " << tracename << endl;
		exit (-1);
	}

        for (int i = 0; i < num_detected_periods; i ++)
	{
	  Periods.push_back(detected_periods[i]);
	}

	for (int i = 0; i < num_tasks; i++)
	{
		unsigned long long PeriodLength = Periods[0]->length * 1000000;

		CurrentPeriod[i] = 0;

		/* Project the starting time of each iteration from the first "best" one */
		CurrentIterTime[i] = Periods[0]->best_ini;
		while (CurrentIterTime[i] >= Periods[0]->ini)
		{
			CurrentIterTime[i] -= PeriodLength;
		}
		CurrentIterTime[i] += PeriodLength;
		/* Now points to the start time of the first iteration in the periodic zone */
	}
}

Process::~Process ()
{
	traceout.close ();
}

void Process::processComment (string &c)
{
	traceout << "#" << c << endl;
}

void Process::processCommunicator (string &c)
{
	traceout << "c" << c << endl;
}

#define ITERATION_MARK_TYPE 666001
#define ITERATION_MARK_VALUE     1

void Process::processState (struct state_t &s)
{
	int                Rank         = s.ObjectID.task - 1;
	unsigned long long StateIni     = s.Begin_Timestamp;
	unsigned long long StateEnd     = s.End_Timestamp;
	unsigned long long PeriodLength = Periods[ CurrentPeriod[Rank] ]->length * 1000000;

	traceout << "1:"
 		 << s.ObjectID.cpu << ":" << s.ObjectID.ptask << ":" << s.ObjectID.task << ":" << s.ObjectID.thread << ":"
		 << s.Begin_Timestamp << ":" << s.End_Timestamp << ":" << s.State << endl; 

	if ((s.State == 1) && (CurrentIterTime[Rank] != -1) && (StateEnd >= CurrentIterTime[Rank]) && (StateIni < Periods[ CurrentPeriod[Rank] ]->end))
	{
	  int value = (CurrentIterTime[Rank] + PeriodLength > Periods[ CurrentPeriod[Rank] ]->end) ? 0 : ITERATION_MARK_VALUE;

	  traceout << "2:"
                   << s.ObjectID.cpu << ":" << s.ObjectID.ptask << ":" << s.ObjectID.task << ":" << s.ObjectID.thread << ":"
		   << s.Begin_Timestamp << ":" << ITERATION_MARK_TYPE << ":" << value << endl;
	  CurrentIterTime[Rank] += PeriodLength;
	}
	
	if ((CurrentIterTime[Rank] != -1) && (CurrentIterTime[Rank] > Periods[ CurrentPeriod[Rank] ]->end))
	{
		CurrentPeriod[Rank] ++;
		if (CurrentPeriod[Rank] > Periods.size() - 1)
			CurrentIterTime[Rank] = -1;
		else
		{
			/* Consider the iterations start with the periodic zone ... this is not true! 
			CurrentIterTime[Rank] = Periods[ CurrentPeriod[Rank] ]->ini; */

			/* Project the starting time of each iteration from the first "best" one */
			CurrentIterTime[Rank] = Periods[ CurrentPeriod[Rank] ]->best_ini;
			while (CurrentIterTime[Rank] >= Periods[ CurrentPeriod[Rank] ]->ini)
			{
				CurrentIterTime[Rank] -= PeriodLength;
			}
			CurrentIterTime[Rank] += PeriodLength;
			/* Now points to the start time of the first iteration in the periodic zone */
		}
	}
} 

void Process::processMultiEvent (struct multievent_t &e)
{
	traceout << "2:" 
		<< e.ObjectID.cpu << ":" << e.ObjectID.ptask << ":" << e.ObjectID.task << ":" << e.ObjectID.thread << ":"
		<< e.Timestamp;
	for (vector<struct event_t>::iterator it = e.events.begin(); it != e.events.end(); it++)
		traceout << ":" << (*it).Type << ":" << (*it).Value;

	traceout << endl;
}

void Process::processEvent (struct singleevent_t &e)
{
}

void Process::processCommunication (struct comm_t &c)
{
	traceout << "3:" 
		<< c.Send_ObjectID.cpu << ":" << c.Send_ObjectID.ptask << ":" << c.Send_ObjectID.task << ":" << c.Send_ObjectID.thread << ":"
		<< c.Logical_Send << ":" << c.Physical_Send << ":"
		<< c.Recv_ObjectID.cpu << ":" << c.Recv_ObjectID.ptask << ":" << c.Recv_ObjectID.task << ":" << c.Recv_ObjectID.thread << ":"
		<< c.Logical_Recv << ":" << c.Physical_Recv << ":"
		<< c.Size << ":" << c.Tag << endl;
}

} /* namespace libparaver */

using namespace::libparaver;
using namespace::std;

int Reconstruct(char *input_trace, int num_detected_periods, Period_t **detected_periods)
{
	int num_tasks = 0;
	string tracename;

	if (input_trace == NULL)
	{
		cerr << "You must provide a tracefile!" << endl;
		return -1;
	}

	tracename = string(input_trace);
	Process *p = new Process (tracename, true, tracename.substr (0, tracename.rfind(".prv"))+string(".iterations.prv"), num_detected_periods, detected_periods);

	p->parseBody();

	/* Copy .pcf and .row files */
	ifstream ifs_pcf ((tracename.substr (0, tracename.rfind(".prv"))+string(".pcf")).c_str());
	if (ifs_pcf.is_open())
	{
		ofstream ofs_pcf ((tracename.substr (0, tracename.rfind(".prv"))+string(".iterations.pcf")).c_str());
		ofs_pcf << ifs_pcf.rdbuf();
		ifs_pcf.close();
		ofs_pcf.close();
	}

	ifstream ifs_row ((tracename.substr (0, tracename.rfind(".prv"))+string(".row")).c_str());
	if (ifs_row.is_open())
	{
		ofstream ofs_row ((tracename.substr (0, tracename.rfind(".prv"))+string(".iterations.row")).c_str());
		ofs_row << ifs_row.rdbuf();
		ifs_row.close();
		ofs_row.close();
	}

	return 0;
}

