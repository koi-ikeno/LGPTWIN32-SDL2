
#include "SDLEventManager.h"
#include "Application/Application.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include "SDLGUIWindowImp.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"
#include <map>

bool SDLEventManager::finished_=false ;
bool SDLEventManager::dumpEvent_=false ;

SDLEventManager::SDLEventManager() 
{
}

SDLEventManager::~SDLEventManager() 
{
}

bool SDLEventManager::Init() 
{
	EventManager::Init() ;
	
	// already init done at WSDLSystem.cpp
	//if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER) < 0 )
	//{
	//	return false;
	//}
  
	
	//SDL_ShowCursor(SDL_DISABLE);
	
	atexit(SDL_Quit) ;
	
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	int joyCount=SDL_NumJoysticks() ;
	joyCount=(joyCount>MAX_JOY_COUNT)?MAX_JOY_COUNT:joyCount ;

	keyboardCS_=new KeyboardControllerSource("keyboard") ;
	const char *dumpIt=Config::GetInstance()->GetValue("DUMPEVENT") ;
	if ((dumpIt)&&(!strcmp(dumpIt,"YES")))
  {
		dumpEvent_=true ;
	}

	for (int i=0;i<MAX_JOY_COUNT;i++) 
  {
		joystick_[i]=0 ;
		buttonCS_[i]=0 ;
		joystickCS_[i]=0 ;
	}
	
	for (int i=0;i<joyCount;i++) 
  {
		char sourceName[128] ;
		joystick_[i]=SDL_JoystickOpen(i) ;
	Trace::Log("EVENT","joystick[%d]=%x",i,joystick_[i]) ;
		Trace::Log("EVENT","Number of axis:%d",SDL_JoystickNumAxes(joystick_[i])) ;
		Trace::Log("EVENT","Number of buttons:%d",SDL_JoystickNumButtons(joystick_[i])) ;
		Trace::Log("EVENT","Number of hats:%d",SDL_JoystickNumHats(joystick_[i])) ;
		sprintf(sourceName,"buttonJoy%d",i) ;
		buttonCS_[i]=new ButtonControllerSource(sourceName) ;
		sprintf(sourceName,"axisJoy%d",i) ;
		joystickCS_[i]=new JoystickControllerSource(sourceName) ;
		sprintf(sourceName,"hatJoy%d",i) ;
		hatCS_[i]=new HatControllerSource(sourceName) ;
	}
  

//  for (int i=0;i<SDLK_LAST;i++) 
//  {
//		keyname_[i]= SDL_GetKeyName((SDLKey)i) ;	
//	} 
  
	return true ;
} 

int SDLEventManager::MainLoop() 
{
	Trace::Log("EVENT","SDLEventManager::MainLoop");
	GUIWindow *appWindow=Application::GetInstance()->GetWindow() ;
	SDLGUIWindowImp *sdlWindow=(SDLGUIWindowImp *)appWindow->GetImpWindow() ;
	while (!finished_)
	{
		SDL_Event event;
		if (SDL_WaitEvent(&event)) 
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (dumpEvent_) 
					{
						Trace::Log("EVENT", "keydown:%s", SDL_GetKeyName(event.key.keysym.sym));
					}
					keyboardCS_->SetKey((int)event.key.keysym.sym,true) ;
					break ;

				case SDL_KEYUP:
					if (dumpEvent_) 
					{
						Trace::Log("EVENT", "keyup:%s", SDL_GetKeyName(event.key.keysym.sym));
					}
					keyboardCS_->SetKey((int)event.key.keysym.sym,false) ;
					break ;


				case SDL_JOYBUTTONDOWN:
					buttonCS_[event.jbutton.which]->SetButton(event.jbutton.button,true) ;
					break ;
				case SDL_JOYBUTTONUP:
					if (dumpEvent_) {
						Trace::Log("EVENT","but(%d):%d",event.button.which,event.jbutton.button) ;
					}
					buttonCS_[event.jbutton.which]->SetButton(event.jbutton.button,false) ;
					break ;
				case SDL_JOYAXISMOTION:
					if (dumpEvent_) {
						Trace::Log("EVENT","joy(%d)::%d=%d",event.jaxis.which,event.jaxis.axis,event.jaxis.value) ;
					}
					joystickCS_[event.jaxis.which]->SetAxis(event.jaxis.axis,float(event.jaxis.value)/32767.0f) ;
					break ;
				case SDL_JOYHATMOTION:
					if (dumpEvent_)
					{
						for (int i=0;i<4;i++)
						{
							int mask = 1<<i ;
							if (event.jhat.value&mask)
							{
								Trace::Log("EVENT","hat(%d)::%d::%d",event.jhat.which,event.jhat.hat,i) ;
							}
						}
					}
					hatCS_[event.jhat.which]->SetHat(event.jhat.hat,event.jhat.value) ;
					break ;
				case SDL_JOYBALLMOTION:
					if (dumpEvent_)
					{
						Trace::Log("EVENT","ball(%d)::%d=(%d,%d)",event.jball.which,event.jball.ball,event.jball.xrel,event.jball.yrel) ;
					}
					break ;
			}

			switch (event.type) 
			{

				case SDL_QUIT:
					sdlWindow->ProcessQuit() ;
					break ;
				//case SDL_VIDEOEXPOSE:
				case SDL_RENDER_TARGETS_RESET:
					sdlWindow->ProcessExpose() ;
					break;
				
				case SDL_USEREVENT:
					sdlWindow->ProcessUserEvent(event) ;
					break ;
			}
		}
	}
	return 0 ;
} ;



void SDLEventManager::PostQuitMessage()
{
  Trace::Log("EVENT","SDEM:PostQuitMessage()") ;
	finished_=true  ;
} ; 

int SDLEventManager::GetKeyCode(const char *key)
{
	using iterator = decltype(SDL2KEYCODEMAP)::iterator;
	
	
	for (int i = 0;i < SDL2KEYCODEMAP.size();i++)
	{
		std::pair<iterator, iterator> ret = SDL2KEYCODEMAP.equal_range(i);
		const char* keyName = SDL_GetKeyName(ret.first->second);
		if ( 0 == strcmp(key, keyName ) )
		{
			return ret.first->second;
		}
	}
	return -1;
}
