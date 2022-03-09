/*
 * HemiMatrixPrep.hpp
 *
 */

#include "HemiMatrixPrep.h"
#include "FHE/Diagonalizer.h"

class CipherPlainMultJob : public ThreadJob
{
public:
    CipherPlainMultJob(vector<Ciphertext>& products,
            const vector<Ciphertext>& multiplicands,
            const vector<Plaintext_<FFT_Data>>& multiplicands2, bool add)
    {
        type = CIPHER_PLAIN_MULT_JOB;
        output = &products;
        input = &multiplicands;
        supply = &multiplicands2;
        length = add;
    }
};

inline void cipher_plain_mult(ThreadJob job, true_type)
{
#ifdef VERBOSE_CIPHER_PLAIN_MULT
    fprintf(stderr, "multiply %d to %d in thread %lx\n", job.begin, job.end,
            pthread_self());
    fflush(stderr);
#endif
    for (int i = job.begin; i < job.end; i++)
    {
        auto prod = ((vector<Ciphertext>*) job.input)->at(i)
                * ((vector<Plaintext_<FFT_Data>>*) job.supply)->at(i);
        auto& results = *((vector<Ciphertext>*) job.output);

        if (job.length)
            results[job.begin] += prod;
        else
            results[i] = prod;

    }
}

inline void cipher_plain_mult(ThreadJob, false_type)
{
    throw not_implemented();
}

class MatrixRandMultJob : public ThreadJob
{
public:
    MatrixRandMultJob(vector<ValueMatrix<gfpvar>>& C,
            const vector<ValueMatrix<gfpvar>>& A,
            vector<ValueMatrix<gfpvar>>& B)
    {
        type = MATRX_RAND_MULT_JOB;
        output = &C;
        input = &A;
        supply = &B;
    }
};

inline void matrix_rand_mult(ThreadJob job, true_type = {})
{
    auto& C = *(vector<ValueMatrix<gfpvar>>*) job.output;
    auto& A = *(vector<ValueMatrix<gfpvar>>*) job.input;
    auto& B = *(vector<ValueMatrix<gfpvar>>*) job.supply;
    SeededPRNG G;

    for (int i = job.begin; i < job.end; i++)
    {
        A[i].randomize(G);
        B[i].randomize(G);
        C[i] = A[i] * B[i];
    }
}

inline void matrix_rand_mult(ThreadJob, false_type)
{
    throw not_implemented();
}

template<class T>
void HemiMatrixPrep<T>::buffer_triples()
{

    assert(prep);
    auto& multipliers = prep->get_multipliers();
    assert(prep->pairwise_machine);
    auto& FTD = prep->pairwise_machine->setup_p.FieldD;
    auto& pk = prep->pairwise_machine->pk;
    int n_matrices = FTD.num_slots() / n_rows;
#ifdef VERBOSE
    fprintf(stderr, "creating %d %dx%d * %dx%d triples\n", n_matrices, n_rows, n_inner,
            n_inner, n_cols);
    fflush(stderr);
    RunningTimer timer;
#endif
    AddableVector<ValueMatrix<gfpvar>> A(n_matrices, {n_rows, n_inner}),
            B(n_matrices, {n_inner, n_cols});
    SeededPRNG G;
    AddableVector<ValueMatrix<gfpvar>> C(n_matrices);
    MatrixRandMultJob job(C, A, B);

    if (BaseMachine::thread_num == 0 and BaseMachine::has_singleton())
    {
        auto& queues = BaseMachine::s().queues;
        int start = queues.distribute(job, n_matrices);
        job.begin = start;
        job.end = n_matrices;
        matrix_rand_mult(job);
        queues.wrap_up(job);
    }
    else
    {
        job.begin = 0;
        job.end = n_matrices;
        matrix_rand_mult(job);
    }

#ifdef VERBOSE_HE
    fprintf(stderr, "encrypt at %f\n", timer.elapsed());
    fflush(stderr);
#endif

    Diagonalizer diag(A, FTD, pk);

    vector<Plaintext_<FFT_Data>> products(n_cols, FTD);
    assert(prep->proc);
    auto& P = prep->proc->P;

    Bundle<octetStream> bundle(P);
    bundle.mine.store(diag.ciphertexts);
    P.unchecked_broadcast(bundle);
    vector<vector<Ciphertext>> others_ct;
    for (auto& os : bundle)
    {
        others_ct.push_back({});
        os.get(others_ct.back(), Ciphertext(pk));
    }

    for (int j = 0; j < n_cols; j++)
        for (auto m : multipliers)
        {
#ifdef VERBOSE
            fprintf(stderr, "column %d with party offset %d at %f\n", j,
                    m->get_offset(), timer.elapsed());
            fflush(stderr);
#endif
            Ciphertext C(pk);
            auto& multiplicands = others_ct[P.get_player(-m->get_offset())];
            if (BaseMachine::thread_num == 0 and BaseMachine::has_singleton())
            {
                auto& queues = BaseMachine::s().queues;
                vector<Ciphertext> products(n_inner, pk);
                vector<Plaintext_<FFT_Data>> multiplicands2;
                for (int i = 0; i < n_inner; i++)
                    multiplicands2.push_back(diag.get_plaintext(B, i, j));
                CipherPlainMultJob job(products, multiplicands, multiplicands2, true);
                int start = queues.distribute(job, n_inner);
#ifdef VERBOSE_HE
                fprintf(stderr, "from %d in central thread\n", start);
                fflush(stderr);
#endif
                for (int i = start; i < n_inner; i++)
                    products[i] = multiplicands.at(i) * multiplicands2.at(i);
                queues.wrap_up(job);
#ifdef VERBOSE_HE
                fprintf(stderr, "adding at %f\n", timer.elapsed());
                fflush(stderr);
#endif
                for (int i = 0; i < n_inner; i++)
                    C += products[i];
            }
            else
                for (int i = 0; i < n_inner; i++)
                    C += multiplicands.at(i) * diag.get_plaintext(B, i, j);

#ifdef VERBOSE_HE
            fprintf(stderr, "adding column %d with party offset %d at %f\n", j,
                    m->get_offset(), timer.elapsed());
            fflush(stderr);
#endif
            m->add(products[j], C, BOTH, n_inner);
        }

    C += diag.dediag(products, n_matrices);

    for (int i = 0; i < n_matrices; i++)
        if (swapped)
            this->triples.push_back(
                    {{B[i].transpose(), A[i].transpose(), C[i].transpose()}});
        else
            this->triples.push_back({{A[i], B[i], C[i]}});

#ifdef VERBOSE_HE
    fprintf(stderr, "done at %f\n", timer.elapsed());
    fflush(stderr);
#endif
}
