# YMYVerb — Vocal Reverb (YMY Records · Made ORYMY)
*The Sweet Space of Bachata.* Reverb vocal **VST3 + Standalone** para FL Studio / Windows, pensado solo para el espacio vocal de la bachata: predelay sincronizado al tempo, colas cálidas y presets por estilo.

## Estado real de esta entrega
| Pieza | Estado |
|---|---|
| Motor DSP (FDN 8 líneas, predelay, sync, ducking, tono, estéreo) | Compila en Windows x64 y **pasa los tests DSP automáticos** |
| 34 presets de fábrica (JSON) | Incluidos; son **puntos de partida por diseño, no ajustes calibrados de oído** |
| Plugin JUCE y editor con skin | Compilados en Release x64; **pluginval strictness 5 pasó**, incluyendo abrir el editor, abrirlo mientras procesa, audio, automatización, estado y buses |
| VST3, aplicación Standalone e instalador | Generados por GitHub Actions y comprobados como archivos de salida |
| Instalación | El Setup instala el VST3 en `C:\Program Files\Common Files\VST3\YMYVerb.vst3` y la aplicación independiente en Program Files |
| Prueba dentro de FL Studio con audio real | **Pendiente de prueba en una instalación de usuario** |

Nota de validación: el subtest interno de `vst3validator` de Steinberg se omite si esa herramienta no está configurada en el entorno de CI. Esto no impidió que la batería de `pluginval` strictness 5 terminara con `SUCCESS`. Los presets y el sonido todavía requieren escucha crítica con voces reales.

## Cómo obtener el .vst3 y el Setup.exe
### Opción A: GitHub Actions
1. En este repositorio, abre la pestaña **Actions** y selecciona **Build YMYVerb (Windows)**.
2. Para repetir la compilación manualmente, pulsa **Run workflow**; también se ejecuta automáticamente al hacer push a la rama principal o a una rama `fix/**`.
3. Descarga el artefacto **YMYVerb-PRO-Windows-x64**. Dentro encontrarás `dist/YMYVerb_Setup.exe`, el ZIP con `YMYVerb.vst3` y `YMYVerb.exe`, y el log de `pluginval`.

El flujo compila Release x64 con Visual Studio 2022, ejecuta los tests DSP, comprueba que existen el bundle VST3 y el Standalone, prueba el plugin con `pluginval` strictness 5 y construye el instalador con Inno Setup 6.

### Opción B: en tu PC
Instala Visual Studio 2022 (carga de trabajo C++), CMake e Inno Setup 6, y ejecuta `scripts\build_windows.bat`. Resultado: `dist\YMYVerb_Setup.exe`. Necesita Internet una vez para bajar JUCE 8.0.4.

El instalador pone el VST3 en `C:\Program Files\Common Files\VST3\YMYVerb.vst3` (requiere administrador por esa carpeta). El Standalone se instala en `Program Files\YMY Records\YMYVerb` con acceso en el menú Inicio. Luego en FL Studio: Options → Manage plugins → Find plugins.

**Standalone:** abre YMYVerb.exe y elige tu interfaz de audio en Options → Audio/MIDI Settings (usa auriculares para evitar acople si activas un micrófono). No tiene host, así que el sync usa el *Manual BPM*.

## Cómo se usa
* **Insert** (en la pista de voz): Dry 100 %, Wet ~25-30 %.  **Aux Send** (canal auxiliar): Dry 0 %, Wet 100 %. Al cambiar el modo en el menú de arriba se aplican esos valores; si estás en Aux con Dry > 0 aparece un aviso.
* **Sync**: el predelay se calcula con el BPM del host (si FL no lo da, usa Manual BPM). Divisiones 1/64 a 1/1, con Triplet o Dotted. Si el resultado pasa de 250 ms se baja por octavas (÷2) para mantenerlo musical. Con Sync apagado, la perilla Predelay es manual (0-250 ms).
* **Micrófono**: se ilumina según el nivel real de entrada (umbral ≈ -50 dB, sube rápido y se apaga suave). Funciona igual en pista o en auxiliar. Botón **LED** lo apaga. No graba ni monitorea audio.
* **Ducking**: baja la reverb mientras hay voz y la deja volver en las pausas (Threshold, Depth, Attack, Release propios).
* **Presets**: categorías, favoritos (Fav), Save/Save As (se guardan en `Documentos\YMY Records\YMYVerb\Presets`, el instalador nunca los toca), A/B, Undo/Redo.
* Perillas: doble clic = valor por defecto.

## Presets (inspirados, NO réplicas)
Romeo Santos · Raulin Rodriguez · Joe Veras (4 cada uno) · Bachata 90s · 2000s · Modern Bachata (largas, 4 cada una) · Aux / Send (10 extra: incluye *Infinite Atmosphere*, decay 14 s). Los nombres de artistas indican el carácter buscado (predelay, calidez, densidad, cola); **no** son sus ajustes reales ni hay vínculo con ellos.

## Pruebas DSP ejecutadas (g++ 13, Linux, `Tests/test_dsp.cpp`)
Silencio→silencio; RT60 medido vs. pedido (Plate/Hall/Hybrid a 1, 2.8 y 6 s: sale ~10 % más corto que lo pedido, ej. 6 s → 5.3-5.5 s); estabilidad 20 s con ruido fuerte y todos los controles al máximo en los 6 algoritmos (sin crecimiento; el limitador interno de seguridad actúa en ese extremo); 5 sample rates (44.1-192 kHz) × 7 tamaños de buffer; predelay (sin fuga antes del tiempo fijado); ducking (-18 dB con voz, cola recuperada en pausas); sync (125 ms, 83.3 ms, 187.5 ms, plegado a 250 ms).
**Aún no probado en condiciones de usuario:** mezcla con voces reales, escucha crítica de los presets, CPU en Ryzen 7 y apertura dentro de FL Studio de tu instalación concreta. `pluginval` sí verificó el editor, el audio, el estado y la automatización; el validador independiente de Steinberg no está configurado en esta CI.

## Estructura
`Source/DSP` (motor y sync, sin JUCE) · `Source/PluginProcessor.*` · `Source/PluginEditor.*` · `Source/Presets/factory_presets.json` (generado con `scripts/gen_presets.py`) · `Tests` · `Installer/YMYVerb.iss` · `.github/workflows/build.yml` · `Resources/Interfaz_referencia.png`.

## La interfaz (skin de tu imagen) y lo que mejoré
La ventana usa tu imagen como fondo (1536×1024, escalable manteniendo proporción). Las perillas, valores, medidores, botones y listas son controles reales dibujados encima; todo lo estático (logo, paneles, etiquetas, ornamentos) viene de la imagen.
Corregido respecto a la imagen: botones con texto corrupto ("Bint", "Reeet") reemplazados por botones reales; título "TONE" cortado redibujado; interruptor Sync que se superponía con su etiqueta; valores y medidores que ahora son en vivo; micrófono apagado en reposo y encendido con la señal de entrada (la capa encendida es tu imagen original, con fundido por nivel).
* El micrófono "apagado" y el fondo de la cápsula están reconstruidos por procesado de imagen: es una aproximación y puede verse distinto a un render real apagado. Si quieres mejor calidad, mándame una versión del micrófono apagado y la uso.
* El icono de candado de Host BPM y el icono de nota de la división no se incluyeron: la división usa los botones Norm / Trip / Dot.
* Botones de la esquina inferior: `Reset` recarga el preset actual, `Init` pone todos los parámetros por defecto, `Scan` relee presets de usuario.

## Limitaciones conocidas
* **Reverb Return** va en dB (−24 a +12), no en %.
* Algoritmos: los 6 comparten el mismo motor FDN con parámetros distintos (tamaño, difusión, modulación, reflexiones, brillo); no son 6 estructuras diferentes.
* Auto Makeup es una compensación simple según el decay. Mono entra y sale mono (suma de L/R del efecto).
* Latencia añadida: 0 muestras. La cola de reverb continúa al detener el transporte.

## Licencias
JUCE 8 se usa bajo **AGPLv3** (o licencia comercial de pago). Si distribuyes el plugin compilado, debes publicar el código fuente bajo AGPLv3 o comprar la licencia comercial de JUCE: https://juce.com/legal/juce-8-licence/ . Inno Setup se usa solo para construir el instalador.
