# Insert Node Dialog Design Plan

## Overview

Design a dialog for the JSON tree editor that allows users to insert different types of JSON nodes: **Object**, **Array**, and **Key-Value Pair** (with subtypes: String, Number, Boolean, Null).

---

## 1. Dialog UI Layout

```
┌─────────────────────────────────────────────┐
│  Insert Node                            [×]  │
├─────────────────────────────────────────────┤
│                                               │
│  Key:   [ ________________ ]                  │
│                                               │
│  Type:  ┌──────────────────────────────┐      │
│         │ ○ Key-Value Pair (String)    │      │
│         │ ○ Key-Value Pair (Number)    │      │
│         │ ○ Key-Value Pair (Boolean)   │      │
│         │ ○ Key-Value Pair (Null)      │      │
│         │ ○ JSON Object    {}          │      │
│         │ ○ JSON Array     []          │      │
│         └──────────────────────────────┘      │
│                                               │
│  Value: [ ________________ ]  ← only for KV  │
│                                               │
│  ┌─────────────────────────────────────────┐  │
│  │ Preview:                                │  │
│  │ "new_key": "new_value"                  │  │
│  └─────────────────────────────────────────┘  │
│                                               │
│         [ Cancel ]        [ OK ]              │
└─────────────────────────────────────────────┘
```

### UI Components

| Component | Type | Description |
|-----------|------|-------------|
| Key Input | QLineEdit | Node key name. Disabled when inserting into Array parent |
| Type Selection | QRadioButton group | 6 options: String, Number, Boolean, Null, Object, Array |
| Value Input | QLineEdit | Node value. Only visible when Key-Value type is selected |
| Boolean Combo | QComboBox | true/false selector. Only visible when Boolean type is selected |
| Preview Panel | QLabel | Real-time preview of the JSON fragment to be inserted |
| OK/Cancel | QPushButton | Confirm or cancel the insertion |

### Dynamic Visibility Rules

- **Key Input**: Disabled when parent is an Array (keys are auto-generated indices)
- **Value Input**: Only shown for String and Number types
- **Boolean Combo**: Only shown for Boolean type (dropdown with true/false)
- **Null**: No value input needed
- **Object/Array**: No value input needed (containers have no direct value)

---

## 2. Interaction Flow

```mermaid
flowchart TD
    A[User right-clicks in edit mode] --> B[Context menu appears]
    B --> C{User clicks}
    C -->|Insert Item| D[Open Insert Node Dialog - sibling mode]
    C -->|Insert Child Item| E[Open Insert Node Dialog - child mode]
    
    D --> F[Dialog shows with position context]
    E --> F
    
    F --> G[User selects type]
    G --> H{Type selected}
    H -->|String/Number| I[Show value input]
    H -->|Boolean| J[Show true/false dropdown]
    H -->|Null| K[No value needed]
    H -->|Object| L[No value needed - will be empty object]
    H -->|Array| M[No value needed - will be empty array]
    
    I --> N[User fills key + value]
    J --> N
    K --> N
    L --> N
    M --> N
    
    N --> O[Preview updates in real-time]
    O --> P{User clicks OK}
    P -->|Cancel| Q[Dialog closes - no changes]
    P -->|OK| R[Validate input]
    
    R --> S{Valid?}
    S -->|Yes| T[Insert node into tree model]
    S -->|No| U[Show error message]
    U --> F
    
    T --> V[Expand tree to show new node]
    V --> W[Select and start editing new node key]
    W --> X[Save config]
    X --> Y[Done]
```

---

## 3. Position Determination Rules

### Insert Item (Sibling)

```mermaid
flowchart TD
    A[Current selection: index] --> B{What is index type?}
    B -->|Object or Array| C[Insert as child of index]
    B -->|String/Number/Bool/Null| D[Insert as sibling - same parent as index]
    B -->|No selection| E[Insert as child of root]
    
    C --> F[New node appended at end of children]
    D --> F
    E --> F
```

### Insert Child Item

```mermaid
flowchart TD
    A[Current selection: index] --> B{Is index valid?}
    B -->|No| C[Show warning: Please select a parent item first]
    B -->|Yes| D{Is index a container?}
    D -->|Not Object/Array| E[Show warning: Cannot insert child into non-container]
    D -->|Object or Array| F{Is parent an Array?}
    
    F -->|Yes| G[Disable key input - auto-generate index key]
    F -->|No| H[Enable key input - user provides key]
    
    G --> I[Insert child at end]
    H --> I
```

### Summary Table

| Scenario | Parent Type | Key Behavior | Position |
|----------|------------|--------------|----------|
| Insert Item, no selection | Root | User enters key | Appended to root |
| Insert Item, leaf selected | Parent of leaf | User enters key | Sibling after leaf |
| Insert Item, container selected | The container | User enters key | Child of container |
| Insert Child, array parent | Array | Auto-generated index | Appended to array |
| Insert Child, object parent | Object | User enters key | Appended to object |
| Insert Child, leaf selected | N/A | Show error | Operation rejected |

---

## 4. QJsonModel Changes Required

The current [`QJsonModel::insertItem()`](third_part/QJsonModel/QJsonModel.cpp:396) hardcodes `QJsonValue::String` type. It needs to accept a type parameter:

```cpp
// New signature
bool insertItem(const QModelIndex &parent, const QString &key, 
                const QVariant &value, QJsonValue::Type type = QJsonValue::String);
```

### Type-specific handling:

| Type | Key | Value | Children |
|------|-----|-------|----------|
| String | User input | User input string | None |
| Number | User input | Parsed as double | None |
| Boolean | User input | true/false | None |
| Null | User input | QJsonValue::Null | None |
| Object | User input | Empty string | Can have children added later |
| Array | Auto-index or user input | Empty string | Can have children added later |

---

## 5. File Changes Summary

### New Files
- `src/widgets/insertnodedialog.h` - Dialog header
- `src/widgets/insertnodedialog.cpp` - Dialog implementation  
- `src/widgets/insertnodedialog.ui` - Dialog UI layout

### Modified Files
- `third_part/QJsonModel/include/QJsonModel.hpp` - Add type parameter to `insertItem()`
- `third_part/QJsonModel/QJsonModel.cpp` - Implement type-aware `insertItem()`
- `src/widgets/jsonfmtwg.cpp` - Replace direct insert with dialog-based insert
- `translations/media-analyzer_zh_CN.ts` - Chinese translations
- `translations/media-analyzer_en_US.ts` - English translations
- `CMakeLists.txt` or `media-analyzer.pro` - Add new source files

---

## 6. Dialog Default Values

| Field | Default | Notes |
|-------|---------|-------|
| Key | `new_key` | For Object parent; disabled for Array parent |
| Type | String | First radio option selected by default |
| Value | `new_value` | For String type |
| Number Value | `0` | For Number type |
| Boolean Value | `true` | For Boolean type |

---

## 7. Validation Rules

1. **Key must not be empty** when parent is Object type
2. **Key must not duplicate** existing sibling keys in the same Object parent
3. **Number value** must be a valid number (double)
4. **Key** should be a valid JSON identifier (no special characters that break JSON structure)
5. When parent is Array, key is auto-generated and validation is skipped
