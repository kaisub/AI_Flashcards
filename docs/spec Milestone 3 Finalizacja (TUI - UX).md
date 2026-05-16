---
tags:
  - architektura
  - plan
  - spec
---

### **Checklista: Finalizacja Milestone 3 (TUI & UX)**

#### **1. Konfiguracja Sesji (Pre-Study Screen)**

Obecnie sesja startuje z domyślnymi ustawieniami. Musimy dodać ekran/okno modalne przed nauką, które pozwoli na:

- [x] **Wybór Kierunku:** (Front -> Back, Back -> Front, Mixed).
    
- [x] **Wybór Sortowania:** (Sekwencyjne - wg listy, Losowe).
    
- [x] **Wybór Trybu:** (Standard SR vs Focused - np. tylko "Nowe").
    

#### **2. Modyfikacje "w locie" (Study Session Upgrades)**

Zgodnie z wymogami UX, użytkownik nie powinien wychodzić z nauki, by poprawić błąd:

- [x] **Przycisk "Edytuj":** Otwiera okno edycji tekstu aktualnej fiszki.
    
- [x] **Przycisk "Usuń":** Usuwa fiszkę z listy i sesji natychmiast.
    
- [x] **Przycisk "Kopiuj":** Wybór docelowej listy dla aktualnej fiszki (np. do "Trudne").
    

#### **3. Zarządzanie Bazą (ListsBrowser Upgrades)**

Obecnie możemy tylko przeglądać i wybierać listy. Musimy dodać operacje na strukturze:

- [x] **Usuwanie Listy/Folderu:** Z potwierdzeniem "Czy na pewno?".
    
- [x] **Zmiana Nazwy:** Wywołanie `IStorage::renameFolder` lub `moveList`.
    
- [x] **Tworzenie Nowej Listy/Folderu:** Bezpośrednio w TUI.
    

#### **4. Operacje Masowe (DeckEditor Upgrades)**

W edytorze brakuje funkcji zarządzania grupami fiszek:

- [x] **Zaznaczanie (Selection):** Możliwość zaznaczenia wielu fiszek na liście (spacja/checkbox).
    
- [x] **Akcje Zbiorcze:** Przenieś zaznaczone / Usuń zaznaczone.
    

#### **5. Ekran Ustawień (Settings)**

- [x] **Wagi SR:** Edycja domyślnych proporcji losowania (obecnie sztywne 70/23/7).

