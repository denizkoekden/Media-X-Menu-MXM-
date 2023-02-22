// XBDialog.h

#ifndef __XBDIALOG_H__
#define __XBDIALOG_H__

#include "xmltree.h"
#include <list>
/*

<Dialog Name="IP">
<Control ID="1" Type="StaticText" X="0" Y="0" H="20" W="60">
 <Text>Enter IP:</Text>
</Control>
</Dialog>

  ALL Controls MUST have an ID, a Parent Dialog, and a Position
 

Create Dialog
Configure Dialog
 AttachDialog to StateApp
  HandleInput
  Render
   Dialog Completes, deleted by StateApp when exiting


CXBDialog is designed to act as a base class for handling a specific dialog.
Derived classes will contain members and functions to handle data and tasks 
specific to the job at hand.
The Derived dialog class will be expected to complete any action started by
whatever launched it.

CXBControl derived components will understand how to draw themselves and handle input
when active. 

Controls that don't have focus will NOT recieve input.


  GetStateApp()->AttachDialog( new CGetIPConfigDlg );

*/

class CXBDialog;


class CXBControl
{
	int				m_iID;
	RECT			m_rect;
	bool			m_bHasFocus;
	bool			m_bIsAttachedToList;
	CXBDialog	*	m_pParentDialog;
public:
	CXBControl();
	~CXBControl();
	virtual HRESULT Render( void );
	virtual HRESULT HandleInput( void );
	virtual	void	OnSetFocus( void );
	virtual	void	OnKillFocus( void );

	CXBControl *	GetPtr( void ) { return this; };
	int				GetID( void )	{	return m_iID; };
	void			SetID( int iID )	{ m_iID = iID; };

	void			Focus( BOOL bFocus );
	virtual	BOOL	HandlesInput( void )	{ return FALSE; };
	void			DeleteAttached( void );

};

typedef list<CXBControl *> TCtrlList;

class CXBDialog
{
protected:
	RECT			m_rect;
	CXBControl	*	m_pControlFocus;
	CXBDialog	*	m_pParent;
	CXBDialog	*	m_pChild;
	TCtrlList		m_ctrlList;
	void			DeleteAttachedControls( void );
public:
	CXBDialog();
	~CXBDialog();
	void	CloseDialog( BOOL bOK );
	BOOL	IsActive( void )	{ return (m_pChild!=NULL); };

	virtual HRESULT Render( void );
	virtual HRESULT	HandleInput( void );
	virtual BOOL OnOK( void );
	virtual void OnCancel( void );
	virtual HRESULT SetFocus( int iCtrlID );
	virtual	HRESULT	OnInitDialog( void );
	BYTE	GlobalAlphaShift( void );
	virtual	void OnChange( int iCtrlID );
	CXBControl * GetControlByID( int iID );
	bool	AttachControl( CXBControl * pControl );
	void	RemoveChildDialog( CXBDialog * pDialog = NULL );
	void	AttachDialog( CXBDialog * pDialog );

};




#endif //  __XBDIALOG_H__