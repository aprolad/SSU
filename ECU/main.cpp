#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <librsync.h>
#include <vector>
#include <chrono>

#define BUFSZ (1024*4)


int delta() {
 
    int e;
    FILE *f;
	FILE *f2;
    char ibuf[BUFSZ];
    char obuf[BUFSZ];
    rs_job_t *job = NULL;
    rs_signature_t *sig = NULL;
    rs_buffers_t rsb = { 0 };
 

    if (!(f = fopen("res.tar", "r"))) {
        perror(NULL);
        return EXIT_FAILURE;
    }
 

	if (!(f2= fopen("delta.txt", "wb"))) {
        perror(NULL);
        return EXIT_FAILURE;
    }
    if (rs_loadsig_file(fopen("sign.txt", "rb"), &sig, NULL) || rs_build_hash_table(sig))
        goto cleanup;

    job = rs_delta_begin(sig);
    do {
        rsb.next_in = ibuf; rsb.avail_in = sizeof(ibuf);
        rsb.avail_in = fread(ibuf, 1, rsb.avail_in, f);
        if (rsb.avail_in == 0) {
            if (ferror(f)) {
                perror(NULL);
                goto cleanup;
            }
            rsb.eof_in = 1;
        }
        do {
            rsb.next_out = obuf, rsb.avail_out = sizeof(obuf);
            e = rs_job_iter(job, &rsb);
            if (e != RS_DONE && e != RS_BLOCKED)
                goto cleanup;
            fwrite(obuf, 1, sizeof(obuf) - rsb.avail_out, f2);
        } while (rsb.avail_in);
    } while (!rsb.eof_in);
 

    std::cout<<"Delta"<<std::endl;
cleanup:
    fclose(f);
	fclose(f2);
    if (job)
        rs_job_free(job);
    if (sig)
        rs_free_sumset(sig);
 
    return rsb.eof_in ? EXIT_SUCCESS : EXIT_FAILURE;
}


int signa() {
    auto start = std::chrono::high_resolution_clock::now();
    int e;
    FILE *f;
	FILE *f2;
    char ibuf[BUFSZ];
    char obuf[BUFSZ];
    rs_job_t *job = NULL;
    rs_signature_t *sig = NULL;
    rs_buffers_t rsb = { 0 };
 

    if (!(f = fopen("test.mp4", "r"))) {
        perror(NULL);
        return EXIT_FAILURE;
    }
 

	if (!(f2= fopen("sigtest.txt", "wb"))) {
        perror(NULL);
        return EXIT_FAILURE;
    }


    job = rs_sig_begin(2048, 0, rs_magic_number(0));
 
    do {
        rsb.next_in = ibuf; ; rsb.avail_in = sizeof(ibuf);
        rsb.avail_in = fread(ibuf, 1, rsb.avail_in, f);
        if (rsb.avail_in == 0) {
            if (ferror(f)) {
                perror(NULL);
                goto cleanup;
            }
            rsb.eof_in = 1;
        }
        do {
            rsb.next_out = obuf, rsb.avail_out = sizeof(obuf);
            e = rs_job_iter(job, &rsb);
            if (e != RS_DONE && e != RS_BLOCKED)
                goto cleanup;
            fwrite(obuf, 1, sizeof(obuf) - rsb.avail_out, f2);

        } while (rsb.avail_in);
    } while (!rsb.eof_in);
 
  

cleanup:
    fclose(f);
	fclose(f2);
    if (job)
        rs_job_free(job);
    if (sig)
        rs_free_sumset(sig);
 
    std::cout<<"Signed"<<std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    long long timePassed = duration.count();
    std::cout << "Time passed: " << timePassed << " milliseconds" << std::endl;
    return rsb.eof_in ? EXIT_SUCCESS : EXIT_FAILURE;
}


int patch() {
 
    int e;
    FILE *f;
	FILE *f2;
    FILE * base;
    char ibuf[BUFSZ];
    char obuf[BUFSZ];
    rs_job_t *job = NULL;
    rs_signature_t *sig = NULL;
    rs_buffers_t rsb = { 0 };


    if (!(f = fopen("delta.txt", "r"))) {
        perror(NULL);
        return EXIT_FAILURE;
    }
 	if (!(base= fopen("test.mp4", "r"))) {
        perror(NULL);
        return EXIT_FAILURE;
    }

	if (!(f2= fopen("result.tar", "wb"))) {
        perror(NULL);
        return EXIT_FAILURE;
    }

    if (rs_loadsig_file(fopen("sigtest.txt", "rb"), &sig, NULL) || rs_build_hash_table(sig))
        goto cleanup;

    job = rs_patch_begin(*rs_file_copy_cb, base);

    do {
        rsb.next_in = ibuf; rsb.avail_in = sizeof(ibuf);
        rsb.avail_in = fread(ibuf, 1, rsb.avail_in, f);
        if (rsb.avail_in == 0) {
            if (ferror(f)) {
                perror(NULL);
                goto cleanup;
            }
            rsb.eof_in = 1;
        }
        do {
            rsb.next_out = obuf, rsb.avail_out = sizeof(obuf);
            e = rs_job_iter(job, &rsb);
            if (e != RS_DONE && e != RS_BLOCKED)
                goto cleanup;
            fwrite(obuf, 1, sizeof(obuf) - rsb.avail_out, f2);
        } while (rsb.avail_in);
    } while (!rsb.eof_in);
 
    std::cout<<"Patched"<<std::endl;

cleanup:
    fclose(f);
	fclose(f2);
    if (job)
        rs_job_free(job);
    if (sig)
        rs_free_sumset(sig);
 
    return rsb.eof_in ? EXIT_SUCCESS : EXIT_FAILURE;
}






int patch_file()
{
    std::cout<<"start patch";
	FILE *basis_file;
	FILE *delta_file;
	rs_result ret;
	FILE *new_file;
	rs_stats_t stats;


	basis_file = rs_file_open("test.mp4", "rb", false);
	delta_file = rs_file_open("delta.txt", "rb", false);
	new_file = rs_file_open("exast.tar", "wb", true);

	ret = rs_patch_file(basis_file, delta_file, new_file, &stats);
	fclose(basis_file);
	fclose(delta_file);
	fclose(new_file);
	if(ret != RS_DONE)
	{
		puts(rs_strerror(ret));
		exit(1);
	}
	rs_log_stats(&stats);
	return 0;
}

int create_delta_file()
{
	FILE *sig_file;
	FILE *new_file;
	FILE *delta_file;
	rs_result ret;
	rs_signature_t *sumset;
	rs_stats_t stats;



	sig_file = fopen("sigtest.txt", "rb");
	new_file = fopen("res.tar", "rb");
	delta_file = fopen("delta.txt", "wb");

	ret = rs_loadsig_file(sig_file, &sumset, &stats);
	if(ret != RS_DONE)
	{
		puts(rs_strerror(ret));
		exit(1);
	}
	rs_log_stats(&stats);

	if(rs_build_hash_table(sumset) != RS_DONE)
	{
		puts(rs_strerror(ret));
		exit(1);
	}

	if(rs_delta_file(sumset, new_file, delta_file, &stats) != RS_DONE)
	{
		puts(rs_strerror(ret));
		exit(1);
	}
	rs_log_stats(&stats);

	rs_free_sumset(sumset);
	fclose(sig_file);
	fclose(new_file);
	fclose(delta_file);
	return 0;
}

int generate_sig()
{
	FILE *basis_file;
	FILE *sig_file;
	size_t block_len = RS_DEFAULT_BLOCK_LEN;
	//size_t strong_len = RS_DEFAULT_STRONG_LEN;
	rs_result ret;
	rs_stats_t stats;


	basis_file = fopen("test.mp4", "rb");
	sig_file = fopen("sigtest.txt", "wb");
	ret = rs_sig_file(basis_file, sig_file, 2048, 0, rs_magic_number(0), &stats);
	fclose(basis_file);
	fclose(sig_file);
	if(ret)
		puts(rs_strerror(ret));
	else
		rs_log_stats(&stats);
	return 0;
}

int main(int argc, char *argv[])
{
    auto start = std::chrono::high_resolution_clock::now();
    signa();
    delta();
	patch();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    long long timePassed = duration.count();
    std::cout << "Time passed: " << timePassed << " milliseconds" << std::endl;


	return 0;
}