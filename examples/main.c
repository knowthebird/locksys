#include "locksys.h"
#include "hal/hal_io.h"
#include "global/user.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main() {
    status_t init_status = locksys_init();

    if (STATUS_ERR_TAMPER == init_status) {
        printf("Tampering Detected, Shutting Down.\n");
        return init_status;
    } else if (STATUS_OK != init_status) {
        printf("Failed to initialize, Shutting Down.\n");
        return init_status;
    }

    char username[64];
    char passphrase[64];

    while (1) {
        printf("\nEnter username (or 'exit' to quit): ");
        fgets(username, sizeof(username), stdin);
        size_t user_len = strnlen(username, MAX_USERNAME_LEN+1);
        if (user_len > 0 && username[user_len - 1] == '\n') {
            username[user_len - 1] = '\0';
        }

        if (strcmp(username, "exit") == 0) {
            break;
        }

        printf("Enter passphrase: ");
        fgets(passphrase, sizeof(passphrase), stdin);
        size_t pass_len = strnlen(passphrase, CONFIG_MAX_PASSWORD_LENGTH+1);
        if (pass_len > 0 && passphrase[pass_len - 1] == '\n') {
            passphrase[pass_len - 1] = '\0';
        }

        status_t auth_status = locksys_open_lock(username, passphrase);

        if (auth_status == STATUS_OK) {
            printf("Lock Opened!\n");

            // Admin menu
            bool is_admin = false;
            if (user_get_is_admin(username, &is_admin) == STATUS_OK && is_admin) {
                printf("\nAdmin Options:\n");
                printf("1. Add new user\n");
                printf("2. Skip\n");
                printf("Select an option (1-2): ");

                char admin_choice[8];
                if (fgets(admin_choice, sizeof(admin_choice), stdin)) {
                    if (admin_choice[0] == '1') {
                        char new_user[64];
                        char new_pass[64];
                        char is_admin_input[8];
                        uint8_t is_admin_flag = 0;

                        printf("Enter new username: ");
                        if (!fgets(new_user, sizeof(new_user), stdin)) {
                            printf("Input error.\n");
                            continue;
                        }
                        size_t len = strnlen(new_user, sizeof(new_user));
                        if (len > 0 && new_user[len - 1] == '\n') {
                            new_user[len - 1] = '\0';
                        }

                        printf("Enter new password: ");
                        if (!fgets(new_pass, sizeof(new_pass), stdin)) {
                            printf("Input error.\n");
                            continue;
                        }
                        len = strnlen(new_pass, sizeof(new_pass));
                        if (len > 0 && new_pass[len - 1] == '\n') {
                            new_pass[len - 1] = '\0';
                        }

                        printf("Is this user an admin? (y/n): ");
                        if (!fgets(is_admin_input, sizeof(is_admin_input), stdin)) {
                            printf("Input error.\n");
                            continue;
                        }
                        if (is_admin_input[0] == 'y' || is_admin_input[0] == 'Y') {
                            is_admin_flag = 1;
                        }

                        status_t add_status = user_add(new_user, new_pass, is_admin_flag);
                        if (add_status == STATUS_OK) {
                            printf("User added successfully.\n");
                        } else {
                            printf("Failed to add user. Status: %d\n", add_status);
                        }
                    }
                }
            }

            printf("Would you like to change your password? (y/n): ");
            char response[4];
            fgets(response, sizeof(response), stdin);
            if (response[0] == 'y' || response[0] == 'Y') {
                char old_pass[64];
                char new_pass[64];

                printf("Re-enter current passphrase to confirm: ");
                fgets(old_pass, sizeof(old_pass), stdin);
                size_t old_len = strnlen(old_pass, CONFIG_MAX_PASSWORD_LENGTH+1);
                if (old_len > 0 && old_pass[old_len - 1] == '\n') {
                    old_pass[old_len - 1] = '\0';
                }

                printf("Enter new passphrase: ");
                fgets(new_pass, sizeof(new_pass), stdin);
                size_t new_len = strnlen(new_pass, CONFIG_MAX_PASSWORD_LENGTH+1);
                if (new_len > 0 && new_pass[new_len - 1] == '\n') {
                    new_pass[new_len - 1] = '\0';
                }

                status_t change_status = locksys_reset_passphrase(username, old_pass, new_pass);
                if (change_status == STATUS_OK) {
                    printf("Password successfully changed.\n");
                } else {
                    printf("Failed to change password. Code: %d\n", change_status);
                }
            }
        } else if ((auth_status == STATUS_ERR_AUTH) || (auth_status == STATUS_ERR_NOT_FOUND)) {
            printf("Authentication Failed.\n");
        } else if (auth_status == STATUS_ERR_PERM_LOCKED) {
            printf("Account permanently locked.\n");
        } else if (auth_status == STATUS_ERR_THROTTLED) {
            printf("Login temporarily disabled. Please wait.\n");
        } else {
            printf("Internal Error. Code: %d\n", auth_status);
        }
    }

    return 0;
}
