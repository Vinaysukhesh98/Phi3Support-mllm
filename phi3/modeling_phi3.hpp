//
// Created by Rongjie Yi on 2024/2/4 0004.
//

#ifndef MODELING_PHI3_HPP
#define MODELING_PHI3_HPP

#include "Layer.hpp"
#include "Module.hpp"
#include "configuration_PHI3.hpp"
#include "models/transformer/modeling_transformer.hpp"

using namespace mllm;

class PHI3MLP final : public Module {
    Layer gate_proj;
    Layer silu;
    Layer up_proj;
    Layer down_proj;

public:
    PHI3MLP() = default;
    PHI3MLP(int hidden_dim, int ffn_hidden, const PHI3NameConfig &names, const string &base_name) {
        gate_proj = Linear(hidden_dim, ffn_hidden, false, base_name + names._gate_proj_name);
        silu = SiLU(base_name + "act");
        up_proj = Linear(hidden_dim, ffn_hidden, false, base_name + names._up_proj_name);
        down_proj = Linear(ffn_hidden, hidden_dim, false, base_name + names._down_proj_name);
    }
    vector<Tensor> Forward(vector<Tensor> inputs, vector<std::any> args) override  {
        auto x = gate_proj(inputs[0]);
        x = silu(x);
        auto y = up_proj(inputs[0]);
        x = x * y;
        x = down_proj(x);
        return {x};
    }
};

class PHI3Block final : public Module {
    MultiHeadAttention attention;
    PHI3MLP mlp;
    Layer norm1;
    Layer norm2;

public:
    PHI3Block() = default;
    PHI3Block(int hidden_dim, int head_size, int ffn_hidden, RoPEType RoPE_type, int cache_limit, const PHI3NameConfig &names, const string &base_name) {
        attention = MultiHeadAttention(hidden_dim, head_size, head_size, hidden_dim / head_size, SPLIT_NONE, false, false,
                                       RoPE_type, cache_limit, true, false, names, base_name + names._attn_base_name);
        mlp = PHI3MLP(hidden_dim, ffn_hidden, names, base_name + names._ffn_base_name);
        norm1 = RMSNorm(hidden_dim, 1e-6, base_name + names._attn_norm_name);
        norm2 = RMSNorm(hidden_dim, 1e-6, base_name + names._ffn_norm_name);
    }
    vector<Tensor> Forward(vector<Tensor> inputs, vector<std::any> args) override  {
        auto x = norm1(inputs[0]);
        x = attention({x, x, x})[0];
        auto tmp = x + inputs[0];
        x = norm2(tmp);
        x = mlp({x})[0];
        x = x + tmp;
        return {x};
    }
};

class PHI3Model final : public Module {
    Layer embedding;
    vector<PHI3Block> blocks;
    Layer norm;
    Layer lm_head;

public:
    explicit PHI3Model(const PHI3Config &config) :
        PHI3Model(config.vocab_size, config.hidden_dim, config.head_size, config.ffn_hidden, config.block_num, config.RoPE_type, config.cache_limit,
                   config.names_config, config.names_config.blk_name) {
    }
    PHI3Model(int vocab_size, int hidden_dim, int head_size, int ffn_hidden, int block_num, RoPEType RoPE_type, int cache_limit,
               const PHI3NameConfig &names, const string &base_name) {
        embedding = Embedding(vocab_size, hidden_dim, names.token_embd_name);
        blocks = List<PHI3Block>(block_num, hidden_dim, head_size, ffn_hidden, RoPE_type, cache_limit, names, base_name);
        norm = RMSNorm(hidden_dim, 1e-6, names.post_norm_name);
        lm_head = Linear(hidden_dim, vocab_size, false, names.lm_head_name);
    }
    vector<Tensor> Forward(vector<Tensor> inputs, vector<std::any> args) override  {
        auto x = embedding(inputs[0]);
        for (auto &block : blocks) {
            x = block({x})[0];
        }
        x = norm(x);
        x = lm_head(x);
        return {x};
    }
};

#endif // MODELING_PHI3_HPP