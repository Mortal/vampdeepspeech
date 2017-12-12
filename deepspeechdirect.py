#!/usr/bin/env python
from timeit import default_timer as timer

import os
import sys

try:
    import numpy as np
except ImportError:
    print("Failed to import Python module numpy", flush=True)
    sys.exit(0)
try:
    from deepspeech.model import Model
except ImportError:
    print("Failed to import Python module deepspeech", flush=True)
    sys.exit(0)

# These constants control the beam search decoder

# Beam width used in the CTC decoder when building candidate transcriptions
BEAM_WIDTH = 500

# The alpha hyperparameter of the CTC decoder. Language Model weight
LM_WEIGHT = 1.75

# The beta hyperparameter of the CTC decoder. Word insertion weight (penalty)
WORD_COUNT_WEIGHT = 1.00

# Valid word insertion weight. This is used to lessen the word insertion penalty
# when the inserted word is part of the vocabulary
VALID_WORD_COUNT_WEIGHT = 1.00


# These constants are tied to the shape of the graph used (changing them changes
# the geometry of the first layer), so make sure you use the same constants that
# were used during training

# Number of MFCC features to use
N_FEATURES = 26

# Size of the context window used for producing timesteps in the input vector
N_CONTEXT = 9


def read_audio():
    chunks = []
    while True:
        n_bin = sys.stdin.buffer.read(4)
        if len(n_bin) < 4:
            print("Unexpected EOF after reading %r" % n_bin, file=sys.stderr)
            break
        n = np.frombuffer(n_bin, np.int32)[0]
        if n == 0:
            break
        float_chunk = np.frombuffer(sys.stdin.buffer.read(n*4), np.float32)
        chunks.append((float_chunk * 2**15).astype(np.int16))
    if chunks:
        return np.concatenate(chunks)


def main():
    base = os.path.dirname(__file__)
    model = os.path.join(base, 'models', 'output_graph.pb')
    alphabet = os.path.join(base, 'models', 'alphabet.txt')
    lm = os.path.join(base, 'models', 'lm.binary')
    trie = os.path.join(base, 'models', 'trie')
    missing = [f for f in (model, alphabet, lm, trie)
               if not os.access(f, os.R_OK)]
    if missing:
        print('Model missing/inaccessible: %r' % missing, flush=True)
        return
    print("OK", flush=True)

    print('Loading model from file %s' % (model), file=sys.stderr)
    model_load_start = timer()
    ds = Model(model, N_FEATURES, N_CONTEXT, alphabet, BEAM_WIDTH)
    model_load_end = timer() - model_load_start
    print('Loaded model in %0.3fs.' % (model_load_end), file=sys.stderr)

    print('Loading language model from files %s %s' % (lm, trie), file=sys.stderr)
    lm_load_start = timer()
    ds.enableDecoderWithLM(alphabet, lm, trie, LM_WEIGHT,
                           WORD_COUNT_WEIGHT, VALID_WORD_COUNT_WEIGHT)
    lm_load_end = timer() - lm_load_start
    print('Loaded language model in %0.3fs.' % (lm_load_end), file=sys.stderr)

    fs = 16000
    while True:
        audio = read_audio()
        if audio is None:
            break
        # We can assume 16kHz
        audio_length = len(audio) * ( 1 / 16000)

        print('Running inference.', file=sys.stderr)
        inference_start = timer()
        inference = ds.stt(audio, fs)
        print(inference, flush=True)
        inference_end = timer() - inference_start
        print('Inference took %0.3fs for %0.3fs audio file.' % (inference_end, audio_length), file=sys.stderr)

if __name__ == '__main__':
    main()
