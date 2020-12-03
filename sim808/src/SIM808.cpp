#include "SIM808.h"
#include <ArduinoLog.h>
#define NL  "\n"

TOKEN(RDY);


SIM808::SIM808(uint8_t resetPin, uint8_t pwrKeyPin, uint8_t statusPin)
{
	Log.notice(S_F(" _0_"));
	_resetPin = resetPin;
	_pwrKeyPin = pwrKeyPin;
	_statusPin = statusPin;

	pinMode(_resetPin, OUTPUT);
	if(_pwrKeyPin != SIM808_UNAVAILABLE_PIN) pinMode(_pwrKeyPin, OUTPUT);
	if (_statusPin != SIM808_UNAVAILABLE_PIN) pinMode(_statusPin, INPUT);
	
	if(_pwrKeyPin != SIM808_UNAVAILABLE_PIN) digitalWrite(_pwrKeyPin, HIGH);
	digitalWrite(_resetPin, HIGH);
}

SIM808::~SIM808() { }

#pragma region Public functions

void SIM808::init()
{
	Log.notice(S_F("1" NL));
	SIM808_PRINT_SIMPLE_P("Init...");
	Log.notice(S_F("2" NL));

	reset();
	Log.notice(S_F("3" NL));
	waitForReady();
	Log.notice(S_F("4" NL));
	delay(1500);
	Log.notice(S_F("5" NL));

	setEcho(SIM808Echo::Off);
	Log.notice(S_F("6" NL));
}

void SIM808::reset()
{
	digitalWrite(_resetPin, HIGH);
	delay(10);
	digitalWrite(_resetPin, LOW);
	delay(200);

	digitalWrite(_resetPin, HIGH);
}

void SIM808::waitForReady()
{
	do
	{
		Log.notice(S_F("%"));
		SIM808_PRINT_SIMPLE_P("Waiting for echo...");
		Log.notice(S_F("Loading"));
		sendAT(S_F(""));
	// Despite official documentation, we can get an "AT" back without a "RDY" first.
	} while (waitResponse(TO_F(TOKEN_AT)) != 0);

	// we got AT, waiting for RDY
	while (waitResponse(TO_F(TOKEN_RDY)) != 0);
}

bool SIM808::setEcho(SIM808Echo mode)
{
	sendAT(S_F("E"), (uint8_t)mode);
	return waitResponse() == 0;
}

size_t SIM808::sendCommand(const char *cmd, char *response, size_t responseSize)
{
	flushInput();
	sendAT(cmd);
	
	uint16_t timeout = SIMCOMAT_DEFAULT_TIMEOUT;
	readNext(response, responseSize, &timeout);
}

#pragma endregion


