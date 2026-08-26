# NOSE - pakiet reprodukowalności pracy magisterskiej

Repozytorium zawiera zanonimizowane dane surowe, markery protokołów, oprogramowanie urządzeń, aplikacje Flutter oraz skrypt umożliwiający odtworzenie tabel i rysunków wykorzystanych w pracy magisterskiej.

## Zakres badania

Badanie miało charakter pilotażowy i zostało wykonane na jednym uczestniku, będącym autorem pracy. Wyników nie należy uogólniać na populację ani interpretować jako walidacji klinicznej. Metronom określał rytm zadany; nie stanowił niezależnego pomiaru rzeczywistej częstości oddechu.

## Struktura

- `data/raw/` - niezmienione pliki CSV i JSON z pięciu sesji;
- `data/derived/` - tabele wynikowe generowane przez skrypt;
- `analysis/` - kanoniczny skrypt analizy i lista zależności;
- `firmware/` - oprogramowanie trzech urządzeń;
- `apps/research_recorder/` - pełny projekt aplikacji do rejestracji badań;
- `apps/live_monitor/` - pełny projekt aplikacji końcowej;
- `releases/` - gotowe pliki APK obu aplikacji oraz ich sumy kontrolne;
- `media/` - film demonstracyjny przedstawiający działanie aplikacji i pasa tensometrycznego;
- `docs/` - opis danych, protokołów, ograniczeń i sposobu odtworzenia wyników;
- `thesis/` - poprawione źródła LaTeX i końcowy PDF pracy.

## Gotowe aplikacje i film

- [NOSE - monitor oddechu](releases/NOSE-monitor-android.apk)
- [NOSE - rejestrator badawczy](releases/NOSE-research-recorder-android.apk)
- [Film demonstracyjny](media/bcb86ff7-6774-403a-926b-30ac6f548d30.mp4)
- [Sumy kontrolne SHA-256](releases/SHA256SUMS.txt)

Pliki APK są przeznaczone do instalacji na urządzeniu z systemem Android. System
może wymagać jednorazowego zezwolenia na instalowanie aplikacji z przeglądarki lub
menedżera plików. Monitor może działać lokalnie bez konta. Funkcje synchronizacji
online wymagają własnego projektu Firebase i ponownego zbudowania aplikacji z
parametrami opisanymi w `apps/live_monitor/README.md`.

## Odtworzenie wyników

1. Utwórz środowisko Python 3.11 lub nowsze.
2. Zainstaluj zależności:

   ```text
   pip install -r analysis/requirements.txt
   ```

3. Uruchom analizę z katalogu głównego repozytorium:

   ```text
   python analysis/reproduce_analysis.py
   ```

Skrypt tworzy tabele CSV i LaTeX w `data/derived/` oraz rysunki PNG w `figures/generated/`.

4. Uruchom testy podstawowych operacji numerycznych:

   ```text
   python -m unittest discover -s analysis -p "test_*.py"
   ```

## Budowanie aplikacji Flutter

Wymagane są Flutter, Android SDK i JDK zgodne z używaną wersją Gradle. W katalogu
wybranej aplikacji należy wykonać:

```text
flutter pub get
flutter analyze
flutter build apk --release
```

Wynik zostanie zapisany jako `build/app/outputs/flutter-apk/app-release.apk`.

## Ważne założenia analizy

- sygnały są interpolowane liniowo na wspólną siatkę 8 Hz przed FFT;
- raportowane `SNR_ref` jest miarą koncentracji energii wokół rytmu zadanego, a nie klasycznym SNR czujnika;
- pasmo referencyjne wynosi `f_ref +/- 0.05 Hz`;
- energia zakłóceń jest liczona we wspólnym zakresie `0.01-2.00 Hz` poza pasmem referencyjnym;
- wariant surowy jest jedynie centrowany przez odjęcie średniej, natomiast wariant `detrend` ma usuwany trend liniowy;
- tabele nie zawierają średnich łączących różne nozdrza ani różne etapy badania;
- sesja stałego metronomu jest wykorzystywana do analizy etapów 6, 12 i 20 oddechów/min.

## Prywatność i przeznaczenie

Pakiet nie zawiera danych umożliwiających identyfikację osób innych niż autor. System jest prototypem badawczym, a nie wyrobem medycznym. Funkcja aplikacji opisana jako alarm braku wykrytego oddechu nie służy do rozpoznawania bezdechu sennego.

## Wykorzystanie GenAI

Podczas porządkowania repozytorium, kontroli spójności, redakcji dokumentacji oraz przygotowania części kodu pomocniczego wykorzystano OpenAI Codex. Autor pracy odpowiada za weryfikację kodu, obliczeń, źródeł i treści pracy.

## Przed publikacją i złożeniem

- prawa do ilustracji zestawiono w `docs/FIGURE_SOURCES.md`;
- kwestie administracyjne wymagające osobistego potwierdzenia opisano w `docs/SUBMISSION_CHECKLIST.md`;
- zakres wykonanej kontroli technicznej znajduje się w `docs/QUALITY_ASSURANCE.md`;
- sposób zamknięcia uwag z recenzji roboczej opisano w `docs/PROMOTER_REPORT_RESOLUTION.md`;
- repozytorium nie ustanawia otwartej licencji; szczegóły zawiera `NOTICE.md`.
