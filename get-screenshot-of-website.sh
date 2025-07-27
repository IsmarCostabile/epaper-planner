# Option 1 (might have issues with js)

wkhtmltoimage \
  --width 800 \
  --height 480 \
  --zoom 0.75 \
  https://wttr.in/ \
  /tmp/pagewk.png

# Option 2 much slower
chromium-browser \   
    --headless \ 
    --disable-gpu \  
    --force-device-scale-factor=0.75 \  
    --window-size=800,480 \  
    --screenshot=/tmp/page.png  \
    --virtual-time-budget=10000 \  
    https://wttr.in