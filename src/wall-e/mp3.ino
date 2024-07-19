DateTime mp3UtilLastCommandDate;
u_long mp3UtilLastCommandMillis;            //this number will overflow after ~49 days
const u_long mp3UtilWaitTimeMillis = 800;   //time to wait between two commandsto the MP3 module

void initMp3Utils() {
  mp3UtilLastCommandDate = DateTime(rtc.now().unixtime() - 2);
  mp3UtilLastCommandMillis = millis();
}

/**
* We must wait at least some time between sending two commands to the MP3 module.
* This thing is crazy slow :-(
* In test-mode this method does not wait, but returns false when required to wait.
* Additionally the test-mode is not considered as a mp3 command.
*/
boolean waitMp3Ready(boolean testMode) {
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
      if(testMode) {
        skipped = true;
      } else {
        Serial.print("Delay due to waiting for MP3 module to accept the next command: ");
        Serial.print(currentWaitTimeMillis);
        Serial.println(" millis");
        delay(currentWaitTimeMillis);
      }
    }
  }

  if(skipped) {
    return false;
  }

  if(!testMode) {
    mp3UtilLastCommandDate = rtc.now();
    mp3UtilLastCommandMillis = millis();
  }

  return true;
}

boolean isMp3Playing() {
  waitMp3Ready(false);
  return myMP3.isPlaying();
}

void setMp3Volume(int volume) {
  waitMp3Ready(false);
  myMP3.volume(volume);
}

void playMp3RandomFile() {
  if(mp3TrackCount > 0) {
    waitMp3Ready(false);
    myMP3.playFolder(1, random(1, mp3TrackCount + 1));
  }
}

void playMp3RandomBootSound() {
  if(mp3TrackCount > 0) {
    waitMp3Ready(false);
    playMp3File(random(1, min(gMp3CountInitSound, mp3TrackCount) + 1));
  }
}

void playMp3RandomAlarm() {
  if(mp3TrackCount > gMp3CountInitSound) {
    waitMp3Ready(false);
    myMP3.playFolder(1, random(gMp3CountInitSound + 1, mp3TrackCount + 1));
  } else {
    //If no other MP3 is avalable, play the boot sounds
    playMp3RandomBootSound();
  }
}

void playMp3File(int fileNumber) {
  if(fileNumber <= mp3TrackCount) {
    waitMp3Ready(false);
    myMP3.playFolder(1, fileNumber);
  }
}

void stopMp3Playback() {
  waitMp3Ready(false);
  myMP3.stop();
}

int getMp3TrackCount() {
  waitMp3Ready(false);
  return myMP3.numSdTracks();
}

int getMp3MaxPossibleVolume() {
  return 30;
}