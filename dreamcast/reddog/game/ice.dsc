//scenes start here

SCENE 0
	SCENESCRIPTENTRY 0
	SCENEENTRYEND
SCENEEND

SCENE 1
	SCENESCRIPTENTRY 1
	SCENEENTRYEND
SCENEEND

SCENE 2
	SCENESCRIPTENTRY 2
	SCENEENTRYEND
SCENEEND

SCENE 3
	SCENESCRIPTENTRY 3
	SCENEENTRYEND
SCENEEND

SCENE 4
	SCENESCRIPTENTRY 4
	SCENEENTRYEND
SCENEEND

SCENE 5
	SCENESCRIPTENTRY 5
	SCENEENTRYEND
SCENEEND

SCENE 6
	SCENESCRIPTENTRY 6
	SCENEENTRYEND
SCENEEND

//text starts here

ENTRY 0
	SCRIPT STOPS THE GAME
	SCRIPT CAN BE SKIPPED
	INTELLIGENCE OFFICER
		TIME 90
		START
			TEXT `ICE_LOCATION
			TEXT `ICE_TIME
			TEXT `ICE_OBJECTIVE
			TEXT `ICE_BRIEFING
		END



ENTRY 1
	SCRIPT STOPS THE GAME
	SCRIPT CAN BE SKIPPED
	INTELLIGENCE OFFICER
		TIME 90
		START
			TEXT That's it, you've taken them all out.  
			TEXT Get outside for the pick-up, I'll meet you there.
		END

ENTRY 2
	SCRIPT STOPS THE GAME
	SCRIPT CAN BE SKIPPED
	INTELLIGENCE OFFICER
		TIME 90
		START
			TEXT ICE_SCRIPT2
		END		

ENTRY 3
	SCRIPT STOPS THE GAME
		TIME 60
		START
			TEXT ICE_SCRIPT3
		END

ENTRY 4
	SCRIPT STOPS THE GAME
		TIME 90
		START
			NOTEXT
		END

ENTRY 5
	SCRIPT STOPS THE GAME
		TIME 75
		START
			NOTEXT
		END

ENTRY 6
	SCRIPT STOPS THE GAME
		TIME 180
		START
			NOTEXT
		END

