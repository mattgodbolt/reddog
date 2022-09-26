# Microsoft Developer Studio Project File - Name="Game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=Game - Win32 Warning
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak" CFG="Game - Win32 Warning"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Game - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Game - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE "Game - Win32 Profile" (based on "Win32 (x86) External Target")
!MESSAGE "Game - Win32 Warning" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Dreamcast/RedDog/Game", KBBAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "Game - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f Game.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Game.exe"
# PROP BASE Bsc_Name "Game.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "NMAKE /f  Game.mak "
# PROP Rebuild_Opt "/a clean"
# PROP Target_File "Game.exe"
# PROP Bsc_Name "Game.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f Game.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Game.exe"
# PROP BASE Bsc_Name "Game.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "NMAKE /f Game.mak _DEBUG="
# PROP Rebuild_Opt "/a clean _DEBUG="
# PROP Target_File "RedDog.elf"
# PROP Bsc_Name "Game.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Game - Win32 Profile"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Game___W"
# PROP BASE Intermediate_Dir "Game___W"
# PROP BASE Cmd_Line "NMAKE /f Game.mak _DEBUG="
# PROP BASE Rebuild_Opt "/a clean _DEBUG="
# PROP BASE Target_File "RedDog.elf"
# PROP BASE Bsc_Name "Game.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Game___W"
# PROP Intermediate_Dir "Game___W"
# PROP Cmd_Line "NMAKE /f Game.mak _PROFILE="
# PROP Rebuild_Opt "/a clean _PROFILE="
# PROP Target_File "RedDog.elf"
# PROP Bsc_Name "Game.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Game - Win32 Warning"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Game___Win32_Warning"
# PROP BASE Intermediate_Dir "Game___Win32_Warning"
# PROP BASE Cmd_Line "NMAKE /f Game.mak _DEBUG="
# PROP BASE Rebuild_Opt "/a clean _DEBUG="
# PROP BASE Target_File "RedDog.elf"
# PROP BASE Bsc_Name "Game.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Game___Win32_Warning"
# PROP Intermediate_Dir "Game___Win32_Warning"
# PROP Cmd_Line "NMAKE /f Game.mak _WARNING="
# PROP Rebuild_Opt "/a clean _WARNING="
# PROP Target_File "RedDog.elf"
# PROP Bsc_Name "Game.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Game - Win32 Release"
# Name "Game - Win32 Debug"
# Name "Game - Win32 Profile"
# Name "Game - Win32 Warning"

!IF  "$(CFG)" == "Game - Win32 Release"

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

!ELSEIF  "$(CFG)" == "Game - Win32 Profile"

!ELSEIF  "$(CFG)" == "Game - Win32 Warning"

!ENDIF 

# Begin Group "Source code"

# PROP Default_Filter "c;asm;cpp"
# Begin Source File

SOURCE=.\backup.c
# End Source File
# Begin Source File

SOURCE=.\camera.c
# End Source File
# Begin Source File

SOURCE=.\coll.c
# End Source File
# Begin Source File

SOURCE=.\collspec.c
# End Source File
# Begin Source File

SOURCE=.\command.c
# End Source File
# Begin Source File

SOURCE=.\debugdraw.c
# End Source File
# Begin Source File

SOURCE=.\draw.c
# End Source File
# Begin Source File

SOURCE=.\FrontEnd.c
# End Source File
# Begin Source File

SOURCE=.\Game.c
# End Source File
# Begin Source File

SOURCE=.\hud.c
# End Source File
# Begin Source File

SOURCE=.\Input.c
# End Source File
# Begin Source File

SOURCE=.\insidepoly.inc
# End Source File
# Begin Source File

SOURCE=.\level.c
# End Source File
# Begin Source File

SOURCE=.\levelst.c
# End Source File
# Begin Source File

SOURCE=.\Loading.c
# End Source File
# Begin Source File

SOURCE=.\Main.c
# End Source File
# Begin Source File

SOURCE=.\Memory.c
# End Source File
# Begin Source File

SOURCE=.\MGMemcpy.src
# End Source File
# Begin Source File

SOURCE=.\MPlayer.c
# End Source File
# Begin Source File

SOURCE=.\pad.c
# End Source File
# Begin Source File

SOURCE=.\Player.c
# End Source File
# Begin Source File

SOURCE=.\process.c
# End Source File
# Begin Source File

SOURCE=.\Procspec.c
# End Source File
# Begin Source File

SOURCE=.\RDDebug.c
# End Source File
# Begin Source File

SOURCE=.\RDInit.c
# End Source File
# Begin Source File

SOURCE=.\sbinit.c
# End Source File
# Begin Source File

SOURCE=.\sndutls.c
# End Source File
# Begin Source File

SOURCE=.\specfx.c
# End Source File
# Begin Source File

SOURCE=.\Strat.c
# End Source File
# Begin Source File

SOURCE=.\StratOldFuncs.c
# End Source File
# Begin Source File

SOURCE=.\StratPhysic.c
# End Source File
# Begin Source File

SOURCE=.\StratWay.c
# End Source File
# Begin Source File

SOURCE=.\Stream.c
# End Source File
# Begin Source File

SOURCE=.\Version.c
# End Source File
# Begin Source File

SOURCE=.\View.c
# End Source File
# Begin Source File

SOURCE=.\Weapon.c
# End Source File
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h;inc"
# Begin Source File

SOURCE=.\afxres.h
# End Source File
# Begin Source File

SOURCE=.\backup.h
# End Source File
# Begin Source File

SOURCE=.\BASICRedDog.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\coll.h
# End Source File
# Begin Source File

SOURCE=.\collspec.h
# End Source File
# Begin Source File

SOURCE=.\command.h
# End Source File
# Begin Source File

SOURCE=.\debugdraw.h
# End Source File
# Begin Source File

SOURCE=.\DebugIO.h
# End Source File
# Begin Source File

SOURCE=.\Dirs.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\GameState.h
# End Source File
# Begin Source File

SOURCE=.\hud.h
# End Source File
# Begin Source File

SOURCE=.\Input.h
# End Source File
# Begin Source File

SOURCE=.\Layers.h
# End Source File
# Begin Source File

SOURCE=.\level.h
# End Source File
# Begin Source File

SOURCE=.\LevelDepend.h
# End Source File
# Begin Source File

SOURCE=.\levelst.h
# End Source File
# Begin Source File

SOURCE=.\Loading.h
# End Source File
# Begin Source File

SOURCE=.\LocalDefs.h
# End Source File
# Begin Source File

SOURCE=.\Macros.h
# End Source File
# Begin Source File

SOURCE=.\Memory.h
# End Source File
# Begin Source File

SOURCE=.\Menu.h
# End Source File
# Begin Source File

SOURCE=.\MPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\process.h
# End Source File
# Begin Source File

SOURCE=.\Procspec.h
# End Source File
# Begin Source File

SOURCE=.\RDDebug.h
# End Source File
# Begin Source File

SOURCE=.\RDInit.h
# End Source File
# Begin Source File

SOURCE=.\RDTypes.h
# End Source File
# Begin Source File

SOURCE=.\RedDog.h
# End Source File
# Begin Source File

SOURCE=.\RedDog.inc
# End Source File
# Begin Source File

SOURCE=.\sndutls.h
# End Source File
# Begin Source File

SOURCE=.\specfx.h
# End Source File
# Begin Source File

SOURCE=.\Strat.h
# End Source File
# Begin Source File

SOURCE=.\StratPhysic.h
# End Source File
# Begin Source File

SOURCE=.\StratWay.h
# End Source File
# Begin Source File

SOURCE=.\Stream.h
# End Source File
# Begin Source File

SOURCE=.\strtcoll.h
# End Source File
# Begin Source File

SOURCE=.\View.h
# End Source File
# Begin Source File

SOURCE=.\VMSStruct.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "mak;txt"
# Begin Source File

SOURCE=.\AdavanceTraining1.DSC
# End Source File
# Begin Source File

SOURCE=.\alien.DSC
# End Source File
# Begin Source File

SOURCE=.\Arena1.log
# End Source File
# Begin Source File

SOURCE=.\Arena13.log
# End Source File
# Begin Source File

SOURCE=.\canyon.dsc
# End Source File
# Begin Source File

SOURCE=.\chal1.dsc
# End Source File
# Begin Source File

SOURCE=.\chal2.dsc
# End Source File
# Begin Source File

SOURCE=.\chal3.dsc
# End Source File
# Begin Source File

SOURCE=.\chal4.dsc
# End Source File
# Begin Source File

SOURCE=.\chal5.dsc
# End Source File
# Begin Source File

SOURCE=.\chal6.dsc
# End Source File
# Begin Source File

SOURCE=.\chal7.dsc
# End Source File
# Begin Source File

SOURCE=.\city.dsc
# End Source File
# Begin Source File

SOURCE=.\credits.txt
# End Source File
# Begin Source File

SOURCE=.\Demoe3.log
# End Source File
# Begin Source File

SOURCE=.\Game.mak
# End Source File
# Begin Source File

SOURCE=.\Ginsu.scr
# End Source File
# Begin Source File

SOURCE=.\hydro.dsc
# End Source File
# Begin Source File

SOURCE=.\hydro.log
# End Source File
# Begin Source File

SOURCE=.\ice.dsc
# End Source File
# Begin Source File

SOURCE=.\Lang.txt
# End Source File
# Begin Source File

SOURCE=.\Level1.cam
# End Source File
# Begin Source File

SOURCE=.\Level1.log
# End Source File
# Begin Source File

SOURCE=.\Level10.dsc
# End Source File
# Begin Source File

SOURCE=.\level2.cam
# End Source File
# Begin Source File

SOURCE=.\Level2.log
# End Source File
# Begin Source File

SOURCE=.\Level3.cam
# End Source File
# Begin Source File

SOURCE=.\level3.dsc
# End Source File
# Begin Source File

SOURCE=.\Level3.log
# End Source File
# Begin Source File

SOURCE=.\Level4.cam
# End Source File
# Begin Source File

SOURCE=.\Level4.log
# End Source File
# Begin Source File

SOURCE=.\Makefile.dep
# End Source File
# Begin Source File

SOURCE=.\mattplevel.cam
# End Source File
# Begin Source File

SOURCE=.\mattplevel.DSC
# End Source File
# Begin Source File

SOURCE=.\Mattplevel.log
# End Source File
# Begin Source File

SOURCE=.\Moog.DSC
# End Source File
# Begin Source File

SOURCE=.\nickc.dsc
# End Source File
# Begin Source File

SOURCE=.\outpost.dsc
# End Source File
# Begin Source File

SOURCE=.\Rules.mak
# End Source File
# Begin Source File

SOURCE=.\savlevel.DSC
# End Source File
# Begin Source File

SOURCE=.\sefton.DSC
# End Source File
# Begin Source File

SOURCE=P:\wads\syswad.txt
# End Source File
# Begin Source File

SOURCE=.\underground.DSC
# End Source File
# Begin Source File

SOURCE=.\volcano.DSC
# End Source File
# Begin Source File

SOURCE=.\VOLCANO.log
# End Source File
# End Group
# Begin Group "Strats"

# PROP Default_Filter "dst"
# Begin Group "SPAWN"

# PROP Default_Filter ""
# Begin Group "Death"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\SPAWN_DEATH_BattleTank1.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_DEATH_GunEmplacementLevel1.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_DEATH_Sefair2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_DEATH_Walker.dst
# End Source File
# End Group
# Begin Group "GEN"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\GEN_BattleTank.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_CaveBot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_FootSoldier.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_FootSoldierLevel2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_HoverBike.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_JETPACKER_Normal.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_Marksman_Footsoldier.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_RapierMini.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_RapierMini_Level2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_Sefair.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_SPAWN_TrainCannister.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GEN_SplineObject.dst
# End Source File
# End Group
# Begin Group "Normal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\Spawn_alienbeam.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_AssaultDroneShot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_auto_enemy_missile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Auto_enemy_shot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_BattleTankShot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_belt.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_blastexp.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_blastrad.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_BlueTrail.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_BOMBER_BOMB.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_BOMBER_WATER_BOMB.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_BulletRing.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_CityBoss.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_collface.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_CrazyFade.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_DarkBullet.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ENEMY_HOMING_MISSILE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_enemy_homing_torpedo.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ENEMY_SHOCKWAVE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ENEMY_VULCAN.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_EnemyDebris.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_EnemyDebrisToCamera.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ENEMYDROPPEDHOMINGMISSILE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_enemymissile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_enemymissileFLAME.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_enemymissilefootsoldier.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_EnemyShot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_EnergyBall.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_EnergyBallTwirl.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ENV_SHOOT_SAT.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExplodingBits.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_EXPLOSION_DEBRIS.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_EXPLOSION_DEBRIS_CAM.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Explosion_Egg.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExplosionBall.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExplosionDome.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExplosionPoly1.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExplosionShard.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExplosionShardGround.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExplosionSpark.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ExpSphere.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_FACE_ALONG_DIR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_FACTORY_SMOKE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_FluffyCloud.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_footSoldier.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_FootSoldierLaserChild.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_FootSoldierLaserParent.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_footSoldierLevel2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_FXTeleport.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_GameCamera.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_GatlinGroundFlare.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_GREEN_TRAIL_ENEMY_MISSILE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_GreenTrail.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_GSPEEDER_MISSILE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_GunTurretMissile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_HeliBeam.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_HomingBullet.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ICE_GUN_LASER.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Ice_Lift_Door.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_Ice_Lift_Section.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ICE_LIFT_TOP.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_IceBaseBeam.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_JetPacker.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_JETTHRUST.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_KillPlayerInOne.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_LaserParent.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_LAVA_BUBBLES.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_LavaRollRock.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_LavaSpray.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_LightFlare.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_LIMBFIRE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_LIMBFIREPARENT.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MainplayerMP.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Mine.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MineDig.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_missile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MissileSmoke.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MissileTwirl.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MissileTwirlMiddle.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MorterShot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MP_Flag.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MultiBotChild.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Multiplayer_Mine.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MultiPlayerHomingBullet.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MultiplayerMissile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_MultiplayerVulcanBullet.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ParamEnemyMissile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_PointToTarget.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_PolyExplosion.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_POLYEXPLOSION_SHOCKWAVE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_PolyExplosionWater.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_PolyWaterSplash.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_PTMD_BULLET.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RAPIER_Mini_Sparkle.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RD_BULLET_4.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RD_PLASMA_1.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RD_PLASMA_2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RD_PLASMA_3.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_rd_v.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RDHM.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RED_LASER_ENEMY.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_ALL_ROUND_SHOCKWAVE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_BEAM.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_BOMB.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_ELECTRO.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_HomingBullet.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_HOVERON.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_LASER.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_ROCKET.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_SWARM.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOG_VULCAN.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RedDogArmageddon.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RedDogDropship_Jet.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_RedDogDropShipThrustFloor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_reddogexhaust.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_REDDOGHUD.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RedDogMissile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RedDogRocket.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RedDogSandTrail.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_RedDogShield.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_RedTrail.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ScapeSmoke.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SCORPIONBALL.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ScorpionBossBeam.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SecondWB.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SHELL.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_ShieldSuckFX.DST
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ShieldTrail.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Shockwave.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SoldierShield.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_splash.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_STARFIRE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SWAMP_HOMING_MISSILE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SwampBossBeam.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SwampProngBeam.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_SWARM.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Trail_fire.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Trailer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_TrainCannister.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Spawn_twister.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_Volcano_LooseRock.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_WaterSlideCamLook.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_WaterWake.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_WhiteTrail.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_YellowBeam.dst
# End Source File
# End Group
# End Group
# Begin Group "ENV"

# PROP Default_Filter ""
# Begin Group "CRANE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\ENV_TRAIN_CRANE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_TRAIN_CRANE_CABLE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_TRAIN_CRANE_HOLDER.dst
# End Source File
# End Group
# Begin Source File

SOURCE=.\Strats\ArmouredCarOnSpline.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_AlarmLights.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Alien_Lift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_alienBoss2Door.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_AlienGeneratorBlastCore.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ALIENMACHINEHUM.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_AnimCanyonBossSFX.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_AnimCityBuildingBlow.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ANIMPLAY.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_AnimPresidentBunkerSuccess1.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Arrow_To_Nearest_Tag.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Beacon.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_BILLYS.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Bird.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_BoatColl.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CamShake.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CanyonBarrier.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CanyonBoat.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CanyonBossDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CanyonBossJabbaDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CanyonBridge.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CanyonBridgeSign.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_CanyonJabbaDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Challenge4_lift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Challenge4Door.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CITY_GLOBE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CityBlastDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CityBunker.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CityBunkerDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CityBunkerDoors.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CityResearchDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_CombatMarine.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_conveyor_belt.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Darken.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DOOR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DoorOpenWaitClose.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DoorUnderWater.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DrawAllTags.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DrawArrowToDest.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DrivingRings.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DROP.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DropGenerator.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_DropShipLWTO.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Environment.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\env_escapesfx.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\env_explosion.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ExplosionSprite.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Factory_Smoke_Generator.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_Fade.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_FALL_DIE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Fan.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_FenceGate.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\env_finalexplosioncontrol.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Floater.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_FXFire.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_FXLavaSmoke.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_FXRain.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_FXSun.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_FXWater.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_General.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\env_genexplosioncontrol.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_GrageDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_GunTurret.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_hexagon_lift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_Hydro_Boss_Doors.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Hydro_Fan.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Hydro_Forcefield.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_HYDRO_LASERFILTER.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Hydro_RisingWater.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Hydro_SpiderCeilingDoors.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_HYDRO_WATER.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_HydroOnBoatSFX.DST
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ICE_FLOOR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Ice_Gun.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_Ice_Life_Gun.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_Ice_lift_lift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_Ice_lift_lift_door.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ICE_LIFT_MANAGER.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ICE_LIFT_SECTION_DOOR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ICE_ROCK_FALL.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ICE_SHIELD_GEN.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ICERIVERSOUND.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_KillPlayerActivation.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_KingOfTheHill.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_LAVA.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_LavaEffects.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_LAVASOUND.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Lift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_LIFT_BOSS.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_LiftOnParentDeath.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Light.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_LightFlare.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Lightning.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_LinkToParentObject.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_MoveAlongPath.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_MP_Lift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_NoMainGun.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ObjectExplodeOnActivation.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ObjectExplodeOnImpact.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ObjectExplodeOnTimer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ObjectExplodeWhenShot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ObjectRotateOnActivation.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_OneLife.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Outpost_MainDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_OUTPOST_Wall.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_OverHere.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_PLATFORM_ROTATE.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_RESTARTPOINT.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Rock.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ROCK_Generator.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Rotate.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_ScorpionIntroSounds.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SEESAW.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SetDestPos.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SetScreenColour.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SewerLight.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SHIELD_DOOR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SHIELD_DOOR_GENERATOR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Spark.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SparkSuck.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SPAWN_GENERIC_Explosion.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Env_SpecialStrat.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SpiderRoomDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Splash.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Sprite.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_STATIC_HICOLL.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_SWAMPS_TELEPORTER.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_TAG.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_TARGET.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_TARGET_GUN.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_TARGET_SHIELD_GENERATOR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_TARGETARROW.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\env_targetboard.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_visibilitycontrol.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Volcano_BarracksDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Volcano_LooseRock.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Volcano_Tremor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_VolcanoBossArea.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_VolcanoCargoLift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_VolcanoEntrance.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_VolcanoExplodingWall.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_WallOfDeathDoor.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Water.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_WaterWheel.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_WeaponEnergy.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_Window.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_WireGate.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENV_WireSparks.dst
# End Source File
# End Group
# Begin Group "STATE_CHANGE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\LevelSettings.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_Deletion.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_DeletionOutsideArea.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_DropshipPickup.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_EndLevel.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_FailChallenge.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_GetOffSwampRaft.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_GetOffTrain.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_GetOnBoat.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_GetOnTrain.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_KillPlayer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_MrDJ.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_Normal.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_PlayerPathControl.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_SetFailure.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_SetPlayerPos.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\StateChange_SetPlayerPosBoost.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_STOPSTRATS.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_TowerLiftPathControl.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_TowerLiftPathControl2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\STATECHANGE_UNTILNOTFAILED.dst
# End Source File
# End Group
# Begin Group "OTHER"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\Aerial_Battletank.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\airfort.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\airlauncher1.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\AlienPowerTube.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\AquaBot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\AssaultDrone.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\AssaultDroneLevel2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Battletank.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\BazookaBoy.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\BigLiftGun.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\BigLiftGunShield.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\BigLiftGunShieldGenerator.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Blockade.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Boat.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Bomber.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\BoxGun.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\CamSimple.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\camTrack.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\carriage.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\CaveBot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ChalTimeLimit.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Coin.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\crawlerGenerator.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\crawlerGenerator123.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\credit.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\CruiseMissile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Deactivator.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\DeletesOnGameOver.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Dummy.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\DummyNoFade.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ENEMY_GENERATOR.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\EscortTanker.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\EscortTankerTail.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\FootSoldier.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\FootSoldierDropship.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\FootSoldierDropshipLevel2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\FootSoldierLevel2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GroundSpeeder.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GunAndGuy.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GunConcealed.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GunEmplacementLevel1.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Gunship.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\GunTurretTracker.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HoldPlayerPlatform.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HotSpot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HoverBike.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HoverBot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HoverBot_Level2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HoverBotCity.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Hydra.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HydroRotatingfilter.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\JETPACKER_Formation.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\JETPACKER_Network.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\JETPACKER_Normal.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\JumpPoint.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Mainplayer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MarineBoy.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MARKSMAN_FootSoldier.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MARKSMAN_FootSoldierLevel2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MARKSMAN_GroundSpeeder.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MINE_PopDown.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MINE_PopUp.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MINE_Proximity.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MINE_Water.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MineAerialLayer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MineDig.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MineDigLayer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MinePopUpGenerator.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MinePopUpGeneratorForest.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MineStaticLayer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MineWaterLayer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MissionBriefing.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MoogTimer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MorterTank.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\mp_Flag.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MultiBot.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Multiplayer.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\MultipleMissileLauncher.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Pickup.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\PopUpGun.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\PresidentsCar.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\raft.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\RAPIER_Big.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\RAPIER_Mini.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\RAPIER_Mini_Level2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ReddogDropship.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\ScoobyDoo.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Score.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Scorpion_Mini.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Script.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SpawnPoint.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SplineObject.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\splineSefair.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SplineSefAir2.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SplineSefAir3.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Sprinkler.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SwampGeneratorShield.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SwampsGeneratorCore.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SwampsGeneratorExit.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SwampsGeneratorPower.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SwampsGeneratorTrigger.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\TowerLift.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Train.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\TriggerStrat.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\TriggerStratsOnActivation.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\Walker.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\WaterMine_Homing.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\WaterMine_Static.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\waterSlide.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\WaterSpeeder.dst
# End Source File
# End Group
# Begin Group "BOSSES"

# PROP Default_Filter ""
# Begin Group "CanyonBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\CanyonBoss.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\CanyonBossDropShip.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\CanyonBossGuide.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_CanyonBossBeam.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_CanyonBossEnergyBall.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_CanyonBossEnergyBallGuide.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_CanyonBossHomingMissile.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_CanyonBossMissile.dst
# End Source File
# End Group
# Begin Group "SwampBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\Swampboss.dst
# End Source File
# End Group
# Begin Group "ScorpionBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\ScorpionBoss.dst
# End Source File
# End Group
# Begin Group "outPostBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\OutPostBoss.dst
# End Source File
# End Group
# Begin Group "hydroBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\HydroBoss.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\HydroSatellite.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_HYDRO_BOSS_BOBBY.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_HydroBoss_MissileA.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_HydroBoss_MissileB.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_HydroBoss_Splash.dst
# End Source File
# End Group
# Begin Group "AlienBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\AlienBoss.dst
# End Source File
# End Group
# Begin Group "CaveBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\CaveBoss.dst
# End Source File
# End Group
# Begin Group "CityBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\CityBossGenerator.dst
# End Source File
# End Group
# Begin Group "iceBoss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\IceBoss.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_IceBossFlames.dst
# End Source File
# Begin Source File

SOURCE=.\Strats\SPAWN_ICEBOSSMUZZLE.dst
# End Source File
# End Group
# Begin Group "Aliengenboss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\aliengenboss.dst
# End Source File
# End Group
# End Group
# Begin Group "Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Strats\DEBUG_Anim.dst
# End Source File
# End Group
# End Group
# Begin Group "Inline graphics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DaveHead.h
# End Source File
# Begin Source File

SOURCE=.\LeonHead.h
# End Source File
# Begin Source File

SOURCE=.\MatPHead.h
# End Source File
# Begin Source File

SOURCE=.\MattHead.h
# End Source File
# Begin Source File

SOURCE=.\NikCHead.h
# End Source File
# Begin Source File

SOURCE=.\NikLHead.h
# End Source File
# Begin Source File

SOURCE=.\VMSLogo.h
# End Source File
# End Group
# Begin Group "IP files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\systemid.src
# End Source File
# Begin Source File

SOURCE=.\zero.src
# End Source File
# End Group
# Begin Group "FrontEnd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\FE.h
# End Source File
# Begin Source File

SOURCE=.\FEBase.cpp
# End Source File
# Begin Source File

SOURCE=.\FEDistract.cpp
# End Source File
# Begin Source File

SOURCE=.\FELayout.rc
# End Source File
# Begin Source File

SOURCE=.\Menus.cpp
# End Source File
# Begin Source File

SOURCE=.\Menus.h
# End Source File
# Begin Source File

SOURCE=.\MiscScreens.cpp
# End Source File
# Begin Source File

SOURCE=.\MiscScreens.h
# End Source File
# Begin Source File

SOURCE=.\MPFrontEnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MPMoreWorkAround.cpp
# End Source File
# Begin Source File

SOURCE=.\MPWorkaround.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SinglePlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\test.cpp
# End Source File
# Begin Source File

SOURCE=.\VMUManager.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\GodMode.scr
# End Source File
# End Target
# End Project
