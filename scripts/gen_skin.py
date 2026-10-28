#!/usr/bin/env python3
"""Builds the GUI skin from Resources/Interfaz_referencia.png (1536x1024):
  Resources/skin_background.png : reference with every dynamic element removed (knobs, values, buttons, lists,
                                  meters, broken/clipped text) and the microphone drawn in its UNLIT state
  Resources/skin_mic_on.png     : the lit microphone (RGBA, feathered); the plugin fades it in with the input level
Needs: python3, numpy, opencv-python, pillow."""
import numpy as np, cv2, sys
from PIL import Image
src = np.array(Image.open("Resources/Interfaz_referencia.png").convert("RGB")).astype(np.float32)
H, W = src.shape[:2]
bg = src.copy()

def fill(x0, y0, x1, y1, img=None, m=10):
    """Replace a rect by bilinear interpolation of the surrounding border (panels are smooth gradients),
    feathered over m px so no seams show."""
    img = bg if img is None else img
    ex0, ey0, ex1, ey1 = max(3, x0 - m), max(3, y0 - m), min(W - 4, x1 + m), min(H - 4, y1 + m)
    L = img[ey0:ey1, ex0 - 3:ex0].mean(axis=1, keepdims=True); R = img[ey0:ey1, ex1:ex1 + 3].mean(axis=1, keepdims=True)
    T = img[ey0 - 3:ey0, ex0:ex1].mean(axis=0, keepdims=True); B = img[ey1:ey1 + 3, ex0:ex1].mean(axis=0, keepdims=True)
    sm = lambda v, ax: cv2.GaussianBlur(v.astype(np.float32), (0, 0), sigmaX=9 if ax == 1 else 0.1, sigmaY=9 if ax == 0 else 0.1, borderType=cv2.BORDER_REPLICATE)
    L, R = sm(L, 0), sm(R, 0); T, B = sm(T, 1), sm(B, 1)      # wash thin features out of the border samples
    tx = np.linspace(0, 1, ex1 - ex0)[None, :, None]; ty = np.linspace(0, 1, ey1 - ey0)[:, None, None]
    interp = (L * (1 - tx) + R * tx + T * (1 - ty) + B * ty) * 0.5
    ax = np.clip(np.minimum(np.arange(ex0, ex1) - ex0 + 1, ex1 - np.arange(ex0, ex1)) / m, 0, 1)
    ay = np.clip(np.minimum(np.arange(ey0, ey1) - ey0 + 1, ey1 - np.arange(ey0, ey1)) / m, 0, 1)
    a = np.minimum(ay[:, None], ax[None, :])[..., None]
    img[ey0:ey1, ex0:ex1] = interp * a + img[ey0:ey1, ex0:ex1] * (1 - a)

def knob(cx, cy, big=True):
    if big: fill(cx - 62, cy - 46, cx + 62, cy + 84)
    else:   fill(cx - 36, cy - 32, cx + 36, cy + 62)

# ---- knobs
for cx in (128, 277, 431): knob(cx, 315)
for cx in (111, 238, 362, 483): knob(cx, 444)
for cx in (102, 226, 361, 498): knob(cx, 637)
for cx in (1067, 1191, 1310, 1430): knob(cx, 287)
for cx in (1081, 1237, 1395): knob(cx, 421)
for cx in (941, 1020, 1101, 1177): knob(cx, 600, False)
knob(962, 702, False)
for cx in (1271, 1336, 1400, 1464): fill(cx - 33, 596, cx + 33, 698)
# ---- other controls
fill(187, 240, 352, 275)                       # algorithm combo
fill(1002, 208, 1070, 240)                     # clipped "TONE" title (redrawn in code)
fill(1038, 678, 1200, 764)                     # ducking / auto makeup toggles
fill(57, 810, 170, 848); fill(186, 815, 278, 850)      # host / manual bpm boxes
fill(306, 788, 396, 862)                       # sync (label overlapped the switch)
fill(56, 894, 200, 931); fill(212, 891, 330, 933)      # division combo + note buttons
fill(358, 896, 452, 934)                       # predelay readout
fill(1136, 782, 1310, 934)                     # button cluster (had corrupted labels)
fill(1338, 799, 1496, 842); fill(1338, 862, 1496, 905) # mode / led buttons
fill(617, 241, 640, 508, m=3); fill(892, 241, 915, 508, m=3)     # meter bars (live in code)
# header: keep logo+title (x<290), clear the rest, per-row colour from the strip at x=290
for y in range(0, 44):
    bg[y, 292:W - 2] = bg[y, 286:290].mean(axis=0)
# preset browser: dark navy gradient (JUCE lists are drawn over it)
px0, py0, px1, py1 = 503, 797, 1110, 968
g = np.linspace(0, 1, py1 - py0)[:, None, None]
bg[py0:py1, px0:px1] = np.array([18, 31, 62], np.float32) * (1 - g) + np.array([10, 18, 40], np.float32) * g

# ---- microphone: rebuild the capsule backdrop WITHOUT mic/glow, bake a dimmed (unlit) mic, keep lit layer separately
RX0, RY0, RX1, RY1 = 672, 50, 860, 728
orig = src[RY0:RY1, RX0:RX1].copy()
# backdrop: per-row pearl colour sampled outside the glow on both sides, smoothed vertically (kills tick streaks)
Lc = src[RY0:RY1, 647:654].mean(axis=1); Rc = src[RY0:RY1, 880:887].mean(axis=1)
row = cv2.GaussianBlur(((Lc + Rc) * 0.5)[:, None, :], (0, 0), 9)[:, 0, :]
backdrop = np.repeat(row[:, None, :], RX1 - RX0, axis=1)
# mic silhouette (image coords), refined by colour
sil = np.zeros(orig.shape[:2], np.uint8)
for (xa, xb, ya, yb) in [(704, 826, 68, 206), (712, 820, 204, 252), (714, 812, 250, 601), (716, 812, 599, 656)]:
    sil[ya - RY0:yb - RY0, xa - RX0:xb - RX0] = 1
hsv0 = cv2.cvtColor(np.clip(orig, 0, 255).astype(np.uint8), cv2.COLOR_RGB2HSV).astype(np.float32)
pearl = (hsv0[..., 1] < 70) & (hsv0[..., 2] > 215)               # pale, bright = capsule/backdrop
mic = (sil == 1) & ~pearl
mic = cv2.morphologyEx(mic.astype(np.uint8), cv2.MORPH_CLOSE, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (13, 13)))
mic = cv2.dilate(mic, np.ones((3, 3), np.uint8)) & sil
mm = cv2.GaussianBlur(mic.astype(np.float32), (0, 0), 1.2)[..., None]
dark = hsv0.copy(); dark[..., 2] = np.minimum(dark[..., 2] * 0.55, 140); dark[..., 1] *= 0.9
mic_off = cv2.cvtColor(np.clip(dark, 0, 255).astype(np.uint8), cv2.COLOR_HSV2RGB).astype(np.float32)
off = mic_off * mm + backdrop * (1 - mm)
fe = np.ones(orig.shape[:2], np.float32); f = 18
for i in range(f):
    a = (i + 1) / f
    fe[i, :] = np.minimum(fe[i, :], a); fe[-1 - i, :] = np.minimum(fe[-1 - i, :], a)
    fe[:, i] = np.minimum(fe[:, i], a); fe[:, -1 - i] = np.minimum(fe[:, -1 - i], a)
fe3 = fe[..., None]
bg[RY0:RY1, RX0:RX1] = off * fe3 + bg[RY0:RY1, RX0:RX1] * (1 - fe3)
rgba = np.dstack([np.clip(orig, 0, 255), fe * 255]).astype(np.uint8)
Image.fromarray(rgba, "RGBA").save("Resources/skin_mic_on.png", optimize=True)
Image.fromarray(np.clip(bg, 0, 255).astype(np.uint8)).save("Resources/skin_background.png", optimize=True)
print("skin written")
