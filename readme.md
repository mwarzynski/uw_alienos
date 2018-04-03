# AlienOS

## Zależności

Aby skompilować emulator na obrazie QEMU z pierwszych zajęć potrzebny jest
zainstalowany cmake w wersji >= 3.0.2.

## Instrukcje kompilacji

Po zainstalowaniu zależności, proces kompilacji emulatora jest standardowy
do każdego z użyciem cmake.

## Opis działania

Działanie emulatora jest zgodne z podaną specyfikacją.

Ponadto, ze względu na wypisywanie komunikatu błędów na stderr, zalecane jest
przekierowanie stderr do pliku aby nie zakłócać pokazywanych treści.
```
./emu prog 1 2> logs
```

W przypadku, gdy emulator wykryje zbyt mały rozmiar terminala aby wyświetlić
treści przekazywane przez syscall print, zostanie wysłany komunikat na stderr,
a treść której nie da się wyświetlić zostanie obcięta. Zatem, w szczególności
gdy wymiary terminala są mniejsze od terminali obcych, można wyświetlać części
treści.

