#include "defines.h"
#include "../include/script.h"
#include "../include/new/helper_functions.h"
#include "../include/new/Vanilla_Functions.h"

//emulate JPANs keypad hack

/*
// called from AgbMain
void InitKeys(void)
{
    gKeyRepeatContinueDelay = 5;
    gKeyRepeatStartDelay = 40;

    gMain.heldKeys = 0;
    gMain.newKeys = 0;
    gMain.newAndRepeatedKeys = 0;
    gMain.heldKeysRaw = 0;
    gMain.newKeysRaw = 0;
}
*/
// values from InitKeys

#define gKeyRepeatContinueDelay 5
#define gKeyRepeatStartDelay 40

#ifdef SAVE_BLOCK_EXPANSION
void TryForcedScript(u8 keyFlag, u16 currKeys) {
	if (!(keyFlag & 0x4))
		return;	// flag not set
	if (gScriptEnv2->enabled)
		return;	// a script is already enabled
	if (gTasks[0].func != (void*) Task_WeatherMain)
		return;
	if ((gKeypadSetter->keyToRunScript & currKeys))
		return;
	script_env_2_enable();
	script_run((void*) gKeypadSetter->scriptToRun);
	return;
};

u16 TryForcedKey(u8 keyFlag, u16 currKeys) {
	if (!(keyFlag & 0x1))
		return currKeys;
	u8 keyCounter = gKeypadSetter->keyForcingCounter;
	if (keyCounter == 0)
		return currKeys;
	gKeypadSetter->keyForcingCounter -= 1;
	return (~(gKeypadSetter->keyMapToForce) & currKeys);
};

u16 TryIgnoringKeys(u8 keyFlag, u16 currKeys) {
	if (!(keyFlag & 0x2))
		return currKeys;
	return (currKeys | gKeypadSetter->keysToIgnore);
};
#endif


// hook at 080005e8 via r0
void ReadKeys(void) {
	
	#ifdef SAVE_BLOCK_EXPANSION
		u16 currKeys = gKeyReg;
		u8 tryKey = gKeypadSetter->keyFlags;
		if (tryKey != 0)
		{
			TryForcedScript(tryKey, currKeys);
			currKeys = TryForcedKey(tryKey, currKeys);
			currKeys = TryIgnoringKeys(tryKey, currKeys);
		}
		u16 keyInput = KEYS_MASK ^ currKeys;
	#else
		u16 keyInput = REG_KEYINPUT ^ KEYS_MASK;
	#endif
    gMain.newKeysRaw = keyInput & ~gMain.heldKeysRaw;
    gMain.newKeys = gMain.newKeysRaw;
    gMain.newAndRepeatedKeys = gMain.newKeysRaw;

    // BUG: Key repeat won't work when pressing L using L=A button mode
    // because it compares the raw key input with the remapped held keys.
    // Note that newAndRepeatedKeys is never remapped either.

    if (keyInput != 0 && gMain.heldKeys == keyInput)
    {
        gMain.keyRepeatCounter--;

        if (gMain.keyRepeatCounter == 0)
        {
            gMain.newAndRepeatedKeys = keyInput;
            gMain.keyRepeatCounter = gKeyRepeatContinueDelay;
        }
    }
    else
    {
        // If there is no input or the input has changed, reset the counter.
        gMain.keyRepeatCounter = gKeyRepeatStartDelay;
    }

    gMain.heldKeysRaw = keyInput;
    gMain.heldKeys = gMain.heldKeysRaw;

    // Remap L to A if the L=A option is enabled.
    if (gSaveBlock2->optionsButtonMode == 2)
    {
        if (gMain.newKeys & L_BUTTON)
            gMain.newKeys |= A_BUTTON;

        if (gMain.heldKeys & L_BUTTON)
            gMain.heldKeys |= A_BUTTON;
    }

    if (gMain.newKeys & gMain.watchedKeysMask)
        gMain.watchedKeysPressed = TRUE;
}




/*	OLD Port
void LoadKeys(void) {
	u16 currKeys = gKeyReg;
	u8 tryKey = KeypadSetter->checkKey;
	//u8 forcedKey = KeypadSetter;	//temp
	
	if (tryKey != 0)
	{
		tryForcedScript(tryKey, currKeys);
		currKeys = tryForcedKey(tryKey, currKeys);
		currKeys = tryIgnoringKeys(tryKey, currKeys);
	}
	u16 allKeys = 0x3FF;
	u16 keyResult = allKeys ^ currKeys;	
	u16 heldButtons = gMain.heldKeysRaw;
	u16 newKeys = ~keyResult & heldButtons;
	gMain.newKeysRaw = newKeys;
	gMain.newKeys = newKeys;
	gMain.newAndRepeatedKeys = newKeys;
	
	u16 loc;
	if ((keyResult == 0) || (gMain.heldKeys != keyResult))
		loc = gKeypadInitialCountdown;
	else
	{
		gMain.keyRepeatCounter -= 1;
		if (gMain.keyRepeatCounter << 16 == 0)
		{
			gMain.heldKeysRaw = keyResult;
			loc = gKeypadFollowUpCountdown;		// b loc_8000636
		}
		// bne loc_800063A
		
	}
	
	if (keyResult != 0)
	{
		if (gMain.heldKeys == keyResult)
		{
			//u16 keyRepeatCounter = gMain->keyRepeatCounter;
			//keyRepeatCounter -= 1;
			//gMain->keyRepeatCounter = keyRepeatCounter;
			
			if (gMain.keyRepeatCounter == 0)
			{
				
				
			}
		}
	}
	//loc_8000634
	
	
	//loc_800063A
	
	
};
*/

