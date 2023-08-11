#include <seal/seal.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fstream>
#include "helpers.h"

using namespace std;
using namespace seal;

int main() {
    // Set up encryption parameters
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

    // Encode and encrypt message
/*     uint64_t message = 10;
    Plaintext plaintext(uint64_to_hex_string(message));
    Ciphertext encrypted;
    encryptor.encrypt(plaintext, encrypted); */
    CKKSEncoder encoder(context);
    double scale = pow(2.0, 40);

    vector<vector<Plaintext>> bids_matrix;
    ifstream file("client_bids.txt");

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string bidder_id;
        double bid, wanted_space;

        getline(iss, bidder_id, ' ');
        iss >> bid >> wanted_space;

        Plaintext bid_pt, wanted_space_pt;
        encoder.encode(bid, scale, bid_pt);
        encoder.encode(wanted_space, scale, wanted_space_pt);

        bids_matrix.push_back({bid_pt, wanted_space_pt});  
    }

    // Send encrypted message to server...
    // Assume socket, sockaddr_in (for the server) are set up here
    // Assume 'sock' is the socket descriptor
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        cout << "Could not create socket";
    }
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed. Error");
        return 1;
    }

    std::stringstream ss;
    /* encrypted.save(ss);

    std::string encrypted_str = ss.str(); */
    for (const auto& row: bids_matrix) {
        for (const auto& plaintext: row) {
            Ciphertext encrypted;
            encryptor.encrypt(plaintext, encrypted);
            encrypted.save(ss);
            //ss << encrypted.save() << "\n";
        }
    }

    uint32_t matrix_size = ss.str().size();

    if(send(sock, &matrix_size, sizeof(matrix_size), 0) < 0) {
        perror("Sending matrix size failed");
        return 1;
    }

    string serialized_matrix = ss.str();

    if(send(sock, &serialized_matrix, sizeof(matrix_size), 0) < 0) {
        perror("Sending matrix content failed");
        return 1;
    }

    // Receive the result from the server
    Ciphertext result;
    //char buffer[4096];
    //memset(buffer, 0, sizeof(buffer));
    if(recv(sock, &ss, sizeof(ss), 0) < 0) {
        perror("Receive failed");
        return 1;
    }

    //ss.str(buffer);
    result.load(context, ss);

    // Decrypt the result
    Plaintext result_plain;
    decryptor.decrypt(result, result_plain);

    // Output the result
    //cout << "The result is: " << result_plain.to_string() << endl;
  
    return 0;
}
