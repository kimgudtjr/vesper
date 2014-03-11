/**----------------------------------------------------------------------------
 * breakpoint.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:2:3 11:29 created
**---------------------------------------------------------------------------*/
#pragma once

#define BACKUP_OPCODE_LENGTH	1

enum BreakPointType
{
	bpt_none, 
	bpt_one_shot,
	bpt_location, 
	bpt_trace
};

enum BreakPointState
{
	bps_none,
//	bps_disabled,
	bps_enabled,
	bps_wait_for_trace,
	bps_unresolved
};

/**
 * @brief	base class that holds breakpoint information
**/
class BreakPoint
{
public:
	BreakPoint();
	virtual ~BreakPoint();

	BreakPointType	get_type()					{ return _type; }
	BreakPointState get_state()					{ return _state; }
	HANDLE			get_single_step_thread()	{ return _single_step_thread; }

	virtual bool handle_breakpoint(_In_ HANDLE hthread, _In_ CONTEXT& context) = 0;
	virtual bool handle_singlestep(_In_ HANDLE hthread, _In_ CONTEXT& context) = 0;

	bool set_breakpoint(_In_ HANDLE hprocess, _In_ DWORD_PTR address);
	bool clear_breakpoint();

	const HANDLE	get_process_handle()	{ return _hprocess; }
	const DWORD_PTR	get_address()			{ return _address; }

protected:
	HANDLE				_hprocess;
	BreakPointType		_type;
	BreakPointState		_state;
	DWORD_PTR			_address;
	unsigned char		_opcode;
	HANDLE				_single_step_thread;
};

/**
 * @brief	class for one shot breakpoint
**/
class OneShotBreakPoint: public BreakPoint
{
public:
	OneShotBreakPoint();
	virtual ~OneShotBreakPoint();

	virtual bool handle_breakpoint(_In_ HANDLE hthread, _In_ CONTEXT& context);
	virtual bool handle_singlestep(_In_ HANDLE hthread, _In_ CONTEXT& context);
};

/**
 * @brief	class for location based breakpoint
**/
class LocationBreakPoint: public BreakPoint
{
public:
	LocationBreakPoint();
	virtual ~LocationBreakPoint();
	virtual bool handle_breakpoint(_In_ HANDLE hthread, _In_ CONTEXT& context);
	virtual bool handle_singlestep(_In_ HANDLE hthread, _In_ CONTEXT& context);
};

/**
 * @brief	class for fast branch trace breakpoint
**/
class TraceBreakPoint: public BreakPoint
{
public:
	TraceBreakPoint();
	virtual ~TraceBreakPoint();
	virtual bool handle_breakpoint(_In_ HANDLE hthread, _In_ CONTEXT& context);
	virtual bool handle_singlestep(_In_ HANDLE hthread, _In_ CONTEXT& context);
};