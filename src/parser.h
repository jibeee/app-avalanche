#pragma once

#include "types.h"
#include "network_info.h"

// some global definitions
enum parse_rv {
    PARSE_RV_INVALID = 0,
    PARSE_RV_NEED_MORE,
    PARSE_RV_PROMPT,
    PARSE_RV_DONE,
};

struct FixedState {
    size_t filledTo;
    uint8_t buffer[1]; // Actually bigger.
};
#define DEFINE_FIXED(name) \
    struct name ## _state { \
        int state; \
        union { \
            name val; \
            uint8_t buf[sizeof(name)]; \
        }; \
    }
#define DEFINE_FIXED_BE(name) \
    struct name ## _state { \
        int state; \
        uint8_t buf[sizeof(name)]; \
        name val; \
    }
#define DEFINE_ARRAY(name) \
    struct name ## s_state { \
        int state; \
        uint32_t len; \
        uint32_t i; \
        union { \
            struct uint32_t_state len_state;\
            struct name ## _state item; \
        }; \
    };

DEFINE_FIXED_BE(uint16_t);
DEFINE_FIXED_BE(uint32_t);
DEFINE_FIXED_BE(uint64_t);

typedef struct {
    uint8_t val[32];
} Id32;

DEFINE_FIXED(Id32);

typedef struct {
    public_key_hash_t val;
} Address;

DEFINE_FIXED(Address);

#define NUMBER_STATES struct uint32_t_state uint32State; struct uint64_t_state uint64State

struct SECP256K1TransferOutput_state {
    int state;
    uint32_t address_n;
    uint32_t address_i;
    union {
        NUMBER_STATES;
        struct Address_state addressState;
    };
};

struct SECP256K1OutputOwners_state {
    int state;
    uint32_t address_n;
    uint32_t address_i;
    union {
        NUMBER_STATES;
        struct Address_state addressState;
    };
};

struct Output_state {
    int state;
    uint32_t type;
    union {
        NUMBER_STATES;
        struct SECP256K1TransferOutput_state secp256k1TransferOutput;
    };
};

struct TransferableOutput_state {
    int state;
    union {
        struct Id32_state id32State;
        struct Output_state outputState;
    };
};

DEFINE_ARRAY(TransferableOutput);

struct SECP256K1TransferInput_state {
    int state;
    uint32_t address_index_n;
    uint32_t address_index_i;
    union {
        NUMBER_STATES;
        struct Address_state addressState;
    };
};

struct Input_state {
    int state;
    uint32_t type;
    union {
        NUMBER_STATES;
        struct SECP256K1TransferInput_state secp256k1TransferInput;
    };
};

struct TransferableInput_state {
    int state;
    union {
        NUMBER_STATES;
        struct Id32_state id32State;
        struct Input_state inputState;
    };
};

DEFINE_ARRAY(TransferableInput);

struct Memo_state {
    int state;
    uint32_t n;
    uint32_t i;
    union {
        NUMBER_STATES;
    };
};

struct BaseTransactionState {
    int state;
    union {
        struct uint16_t_state uint16State;
        struct uint32_t_state uint32State;
        struct Id32_state id32State;
        struct TransferableOutputs_state outputsState;
        struct TransferableInputs_state inputsState;
        struct Memo_state memoState;
    };
};

struct ImportTransactionState {
  int state;
  union {
        struct uint32_t_state uint32State;
        struct Id32_state id32State;
        struct TransferableInputs_state inputsState;
  };
};

struct ExportTransactionState {
  int state;
  union {
        struct uint32_t_state uint32State;
        struct Id32_state id32State;
        struct TransferableOutputs_state outputsState;
  };
};

struct Validator_state {
  int state;
  union {
    struct Address_state addressState;
    struct uint64_t_state uint64State;
  };
};

struct AddValidatorTransactionState {
  int state;
  union {
        struct uint32_t_state uint32State;
        struct Id32_state id32State;
        struct TransferableOutputs_state outputsState;

        struct Validator_state validatorState;
        struct SECP256K1OutputOwners_state ownersState;
  };
};

struct AddDelegatorTransactionState {
  int state;
  union {
        struct uint32_t_state uint32State;
        struct Id32_state id32State;
        struct TransferableOutputs_state outputsState;
  };
};

struct TransactionState {
  int state;
  uint32_t type;
  cx_sha256_t hash_state;
  union {
    struct uint16_t_state uint16State;
    struct uint32_t_state uint32State;
    struct BaseTransactionState baseTxState;
    struct ImportTransactionState importTxState;
    struct ExportTransactionState exportTxState;
    struct AddValidatorTransactionState addValidatorTxState;
    struct AddDelegatorTransactionState addDelegatorTxState;
  };
};

typedef struct {
    uint8_t const *src;
    size_t consumed;
    size_t length;
} parser_input_meta_state_t;

typedef struct {
    uint64_t amount;
    network_id_t network_id;
    Address address;
} output_prompt_t;

typedef struct {
    network_id_t network_id;
    Address address;
} address_prompt_t;

typedef struct {
    string_generation_callback to_string;
    union {
        char const *str; // pointer to static null-terminated string
        uint32_t uint32; // network
        uint64_t uint64; // amount / fee
        Address address;
        //TODO: Now that weve added this, do we need the ones above?
        output_prompt_t output_prompt;
    } data;
} prompt_entry_t;

#define TRANSACTION_PROMPT_BATCH_SIZE 1

enum transaction_type_id_t {
    TRANSACTION_TYPE_ID_BASE = 0,
    TRANSACTION_TYPE_ID_IMPORT = 3,
    TRANSACTION_TYPE_ID_EXPORT = 4,
    TRANSACTION_TYPE_ID_ADD_VALIDATOR = 0x0c,
    TRANSACTION_TYPE_ID_ADD_DELEGATOR = 0x0e,
    TRANSACTION_TYPE_ID_PLATFORM_IMPORT = 0x11,
    TRANSACTION_TYPE_ID_PLATFORM_EXPORT = 0x12
};

typedef struct {
    parser_input_meta_state_t input;
    struct {
        size_t count;
        char const *labels[TRANSACTION_PROMPT_BATCH_SIZE + 1]; // For NULL at end
        prompt_entry_t entries[TRANSACTION_PROMPT_BATCH_SIZE];
    } prompt;
    enum transaction_type_id_t type_id;
    bool swap_output;
    uint64_t last_output_amount;
    network_id_t network_id;
    bool first_asset_id_found;
    Id32 first_asset_id;
    uint64_t sum_of_inputs;
    uint64_t sum_of_outputs;
} parser_meta_state_t;

char const *network_id_string(network_id_t const network_id);
Id32 const *blockchain_id_for_network(network_id_t const network_id);

void initTransaction(struct TransactionState *const state);

enum parse_rv parseTransaction(struct TransactionState *const state, parser_meta_state_t *const meta);
