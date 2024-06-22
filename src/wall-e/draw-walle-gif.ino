void drawWalleGifMain() {
      if (gif.open((uint8_t *)WALLE_GIF_MAIN, sizeof(WALLE_GIF_MAIN), GIFDraw)) {
        tft.startWrite();  // The TFT chip select is locked low
        while (gif.playFrame(true, NULL)) {
          yield();
        }
        gif.close();
        tft.endWrite();  // Release TFT chip select for other SPI devices
      }
}