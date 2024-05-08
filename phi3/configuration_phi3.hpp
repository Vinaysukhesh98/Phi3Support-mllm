//
// Created by Rongjie Yi on 2024/2/4 0004.
//

#ifndef CONFIG_PHI3_HPP
#define CONFIG_PHI3_HPP
#include "models/transformer/configuration_transformer.hpp"

using namespace mllm;

class PHI3NameConfig : public TransformerNameConfig {
public:
    std::string blk_name;
    std::string token_embd_name;
    std::string post_norm_name;
    std::string lm_head_name;
    std::string _gate_proj_name;

    void init(RoPEType type = PHI3ROPE) {
        switch (type) {
        case PHI3ROPE: {
            blk_name = "layers.";
            _attn_base_name = "attention.";
            _ffn_base_name = "feed_forward.";
            _q_proj_name = "wq";
            _k_proj_name = "wk";
            _v_proj_name = "wv";
            _o_proj_name = "wo";
            _gate_proj_name = "w1";
            _up_proj_name = "w3";
            _down_proj_name = "w2";
            _attn_norm_name = "attention_norm";
            _ffn_norm_name = "ffn_norm";
            token_embd_name = "tok_embeddings";
            post_norm_name = "norm";
            lm_head_name = "output";
            break;
        }
        case HFHUBROPE: {
            blk_name = "model.layers.";
            _attn_base_name = "self_attn.";
            _ffn_base_name = "mlp.";
            _q_proj_name = "q_proj";
            _k_proj_name = "k_proj";
            _v_proj_name = "v_proj";
            _o_proj_name = "o_proj";
            _gate_proj_name = "gate_proj";
            _up_proj_name = "up_proj";
            _down_proj_name = "down_proj";
            _attn_norm_name = "input_layernorm";
            _ffn_norm_name = "post_attention_layernorm";
            token_embd_name = "model.embed_tokens";
            post_norm_name = "model.norm";
            lm_head_name = "lm_head";
            break;
        }
        default: {
            throw std::runtime_error("Unsupported PHI3 type");
        }
        }
    }
};

class PHI3Config {
public:
    int vocab_size{};
    int hidden_dim{};
    int head_size{};
    int ffn_hidden{};
    int block_num{};
    RoPEType RoPE_type;
    int cache_limit{};
    PHI3NameConfig names_config;

    explicit PHI3Config(int token_limit, string billions = "7B", RoPEType type = PHI3ROPE, int vocab = 32000) {
        names_config.init(type);
        vocab_size = vocab;
        if (billions == "7B" || billions == "7b") {
            hidden_dim = 4096;
            head_size = 32;
            ffn_hidden = 11008;
            block_num = 32;
        } else {
            throw std::runtime_error("Unsupported model size");
        }
        RoPE_type = type;
        cache_limit = token_limit;
    }
};

#endif // CONFIG_PHI3_HPP
