DateTime mp3UtilLastCommandDate;
long mp3UtilLastCommandMillis;
const u_long mp3UtilWaitTimeMillis = 900;   //time to wait between two commands to the MP3 module

void initMp3Utils() {
  mp3UtilLastCommandDate = DateTime(rtc.now().unixtime() - 2);
  mp3UtilLastCommandMillis = millis();
}

/**
* We must wait at least some time between sending two commands to the MP3 module.
* This thing is crazy slow :-(
*/
boolean testMp3Ready(boolean waitUntilReady) {
  boolean skipped = false;
  DateTime now = rtc.now();
  int timeDiffSeconds = now.unixtime() - mp3UtilLastCommandDate.unixtime();
  if(timeDiffSeconds < 1) {
    //we must wait some millis
    u_long nowMillis = millis();
    u_long timeDiffMillis = nowMillis - mp3UtilLastCommandMillis;

    u_long currentWaitTimeMillis = mp3UtilWaitTimeMillis;
    if(timeDiffMillis > 0) {
      currentWaitTimeMillis = mp3UtilWaitTimeMillis - timeDiffMillis;
    }

    if(currentWaitTimeMillis > 0 && currentWaitTimeMillis <= mp3UtilWaitTimeMillis) {
      if(! waitUntilReady) {
        return false;
      } else {
        Serial.print("Delay due to waiting for MP3 module to accept the next command: ");
        Serial.print(currentWaitTimeMillis);
        Serial.println(" millis");
        delay(currentWaitTimeMillis);
      }
    }
  }

  return true;
}

void mp3CommandExecuted() {
  mp3UtilLastCommandDate = rtc.now();
  mp3UtilLastCommandMillis = millis();
}

boolean isMp3Playing() {
  testMp3Ready(true);
  boolean playing =  myMP3.isPlaying();
  mp3CommandExecuted();
  return playing;
}

void setMp3Volume(int volume) {
  testMp3Ready(true);
  myMP3.volume(volume);
  mp3CommandExecuted();
}

void playMp3RandomFile() {
  if(mp3TrackCount > 0) {
    testMp3Ready(true);
    myMP3.playFolder(1, random(1, mp3TrackCount + 1));
    mp3CommandExecuted();
  }
}

void playMp3RandomBootSound() {
  if(mp3TrackCount > 0) {
    testMp3Ready(true);
    playMp3File(random(1, min(gMp3CountInitSound, mp3TrackCount) + 1));
    mp3CommandExecuted();
  }
}

void playMp3RandomAlarm() {
  if(mp3TrackCount > gMp3CountInitSound) {
    testMp3Ready(true);
    myMP3.playFolder(1, random(gMp3CountInitSound + 1, mp3TrackCount + 1));
    mp3CommandExecuted();
  } else {
    //If no other MP3 is avalable, play the boot sounds
    playMp3RandomBootSound();
  }
}

void playMp3File(int fileNumber) {
  if(fileNumber <= mp3TrackCount) {
    testMp3Ready(true);
    myMP3.playFolder(1, fileNumber);
    mp3CommandExecuted();
  }
}

void stopMp3Playback() {
  testMp3Ready(true);
  myMP3.stop();
  mp3CommandExecuted();
}

int getMp3TrackCount() {
  testMp3Ready(true);
  int num = myMP3.numSdTracks();
  mp3CommandExecuted();
  return num;
}

int getMp3MaxPossibleVolume() {
  return 30;
}