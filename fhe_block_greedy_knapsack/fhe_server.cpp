#include <seal/seal.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fstream>


using namespace std;
using namespace seal;

int main() {
    // Set up encryption parameters
    // This should be identical to the client side
    EncryptionParameters parms(scheme_type::ckks);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));

    SEALContext context(parms);

    // Key generation
    KeyGenerator keygen(context);
    PublicKey public_key;
    SecretKey secret_key = keygen.secret_key();
    keygen.create_public_key(public_key);

    // Set up Encryptor, Decryptor, and Evaluator
    Encryptor encryptor(context, public_key);
    Decryptor decryptor(context, secret_key);
    Evaluator evaluator(context);
    CKKSEncoder encoder(context);
    double scale = pow(2.0, 40);

    // Receive encrypted message from client...
    // Assume 'sock' is the socket descriptor, and 'client' is a sockaddr_in struct
    int sock, new_sock;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(struct sockaddr_in);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        cout << "Could not create socket";
    }
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    if(bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(sock, 3);

    new_sock = accept(sock, (struct sockaddr*)&client, &c);
    if(new_sock < 0) {
        perror("Accept failed");
        return 1;
    }

    //Ciphertext encrypted;

    uint32_t matrix_size;
    if(recv(new_sock, &matrix_size, sizeof(matrix_size), 0) < 0) {
        perror("Receiving matrix size failed");
        return 1;
    }

    std::stringstream ss;

    if(recv(new_sock, &ss, matrix_size, 0) < 0) {
        perror("Receiving matrix content failed");
        return 1;
    }

    istringstream serialized_matrix(ss.str());
    vector<vector<Ciphertext>> bids_matrix(2, vector<Ciphertext>(3));
    for (auto& row: bids_matrix) {
        for (Ciphertext ciphertext: row) {
            //string ser_pt;
            //getline(serialized_matrix, ser_pt);
            ciphertext.load(context, serialized_matrix);
        }
    }

    for (const auto& row: bids_matrix) {
        for (const auto&ct : row) {
            cout << ct.data() << "\n";
        }
    }

/*     // ss.str(buffer);
    encrypted.load(context, ss);

    // Perform operation on the encrypted data
    // For instance, we'll square the input
    */
    Ciphertext result;
    //evaluator.square(encrypted, result);
 
    // Send the result back to the client
    result.save(ss);
    std::string result_str = ss.str();
    if(send(new_sock, &result_str, sizeof(result_str), 0) < 0) {
        perror("Send failed");
        return 1;
    }
 
    return 0;
}
