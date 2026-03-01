#include <stdio.h>        // Entrada e saída
#include <string.h>       // Manipulação de strings
#include <stdlib.h>       // Funções gerais: atoi, atof, malloc etc.
#include <ctype.h>        // Funções de caracteres: tolower, toupper etc.
#include <locale.h>       // Configuração de idioma

#ifdef _WIN32 
#include <windows.h>      // Para configurar UTF-8 no terminal do Windows
#endif

/* ---------- limites ---------- */
#define MAX 200           // Máximo de alunos armazenados
#define MAX_CHAR 128      // Tamanho máximo das strings
#define MAX_MATERIAS 20   // Máximo de matérias por aluno
#define BUF_SIZE 512      // Buffer de leitura de arquivo

/* ---------- estruturas ---------- */
typedef struct {
    char materia[MAX_CHAR];   // Nome da matéria
    float n1;                 // Nota 1
    float n2;                 // Nota 2
    float media;              // Média calculada
} Nota;

typedef struct {
    char nome[MAX_CHAR];      // Nome do aluno
    char ra[MAX_CHAR];        // RA único
    char email[MAX_CHAR];     // E-mail
    char turma[MAX_CHAR];     // Turma (ex: 1A, 2B etc.)
    Nota notas[MAX_MATERIAS]; // Vetor com notas do aluno
    int qtdNotas;             // Quantidade de matérias cadastradas
} Aluno;

/* ---------- arquivos ---------- */
const char *ARQ_ALUNOS = "alunos.csv"; // Caminho do CSV de alunos
const char *ARQ_NOTAS  = "notas.csv";  // Caminho do CSV de notas

/* ---------- memória ---------- */
Aluno alunos[MAX];    // Vetor com todos os alunos carregados
int qtdAlunos = 0;    // Quantidade de alunos cadastrados

/* ---------- utilitários ---------- */

// Remove espaços e quebras de linha no início e fim da string
void trim(char *s) {
    if (!s) return;
    int len = (int)strlen(s);

    // Remove espaços e quebras no final
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' ' || s[len-1] == '\t')) {
        s[--len] = '\0';
    }

    // Remove espaços do começo
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;

    // Move string corrigida para o início
    if (start > 0) memmove(s, s + start, strlen(s + start) + 1);
}

// Comparação de strings ignorando maiúsculas/minúsculas
int strcmpi_simple(const char *a, const char *b) {
    if (!a || !b) return (a==b)?0:1;

    while (*a && *b) {
        char ca = tolower((unsigned char)*a);
        char cb = tolower((unsigned char)*b);

        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        a++; b++;
    }

    return (unsigned char)tolower((unsigned char)*a) - (unsigned char)tolower((unsigned char)*b);
}

// Verifica se "needle" está contido dentro de "hay", ignorando maiúsculas/minúsculas
int contains_ci(const char *hay, const char *needle) {
    if (!hay || !needle) return 0;
    size_t h = strlen(hay), n = strlen(needle);
    if (n == 0) return 1;

    for (size_t i = 0; i + n <= h; i++) {
        size_t j;
        for (j = 0; j < n; j++)
            if (tolower((unsigned char)hay[i+j]) != tolower((unsigned char)needle[j])) break;

        if (j == n) return 1;
    }
    return 0;
}

// Procura aluno pelo RA
int find_aluno_por_ra(const char *ra) {
    if (!ra) return -1;
    for (int i = 0; i < qtdAlunos; i++)
        if (strcmpi_simple(alunos[i].ra, ra) == 0) return i;
    return -1;
}

// Zera todas as notas antes de carregar novamente
void limpar_notas_todas() {
    for (int i = 0; i < qtdAlunos; i++) alunos[i].qtdNotas = 0;
}

/* ---------- leitura/escrita CSV ---------- */

// Lê arquivo alunos.csv
int carregar_alunos_csv() {
    FILE *f = fopen(ARQ_ALUNOS, "r");
    if (!f) return 0;

    char line[BUF_SIZE];
    int linha = 0;

    while (fgets(line, sizeof(line), f)) {
        linha++;
        trim(line);

        if (line[0] == '\0') continue;

        // Ignora cabeçalho
        if (linha == 1 && contains_ci(line, "Nome") && contains_ci(line, "RA"))
            continue;

        // Tokeniza a linha do CSV
        char *tok = strtok(line, ",");

        if (!tok) continue;
        char nome[MAX_CHAR]; strncpy(nome, tok, MAX_CHAR-1); nome[MAX_CHAR-1] = '\0'; trim(nome);

        tok = strtok(NULL, ","); if (!tok) continue;
        char ra[MAX_CHAR]; strncpy(ra, tok, MAX_CHAR-1); ra[MAX_CHAR-1] = '\0'; trim(ra);

        tok = strtok(NULL, ","); if (!tok) continue;
        char email[MAX_CHAR]; strncpy(email, tok, MAX_CHAR-1); email[MAX_CHAR-1] = '\0'; trim(email);

        tok = strtok(NULL, ",");
        char turma[MAX_CHAR] = "";
        if (tok) { strncpy(turma, tok, MAX_CHAR-1); turma[MAX_CHAR-1] = '\0'; trim(turma); }

        // Verifica se RA já existe
        int idx = find_aluno_por_ra(ra);

        if (idx == -1) {
            if (qtdAlunos >= MAX) continue;
            idx = qtdAlunos++;
        }

        // Armazena os dados
        strncpy(alunos[idx].nome, nome, MAX_CHAR-1);
        strncpy(alunos[idx].ra, ra, MAX_CHAR-1);
        strncpy(alunos[idx].email, email, MAX_CHAR-1);
        strncpy(alunos[idx].turma, turma, MAX_CHAR-1);
        alunos[idx].qtdNotas = 0;
    }

    fclose(f);
    return qtdAlunos;
}

// Lê arquivo notas.csv
int carregar_notas_csv() {
    FILE *f = fopen(ARQ_NOTAS, "r");
    if (!f) return 0;

    char line[BUF_SIZE];
    int linha = 0;

    limpar_notas_todas();

    while (fgets(line, sizeof(line), f)) {
        linha++;
        trim(line);

        if (line[0] == '\0') continue;

        if (linha == 1 && contains_ci(line, "RA") && contains_ci(line, "Materia"))
            continue;

        char *tok = strtok(line, ",");

        if (!tok) continue;
        char ra[MAX_CHAR]; strncpy(ra, tok, MAX_CHAR-1); ra[MAX_CHAR-1] = '\0'; trim(ra);

        tok = strtok(NULL, ","); if (!tok) continue;
        char materia[MAX_CHAR]; strncpy(materia, tok, MAX_CHAR-1); materia[MAX_CHAR-1] = '\0'; trim(materia);

        tok = strtok(NULL, ","); if (!tok) continue;
        float n1 = (float)atof(tok);

        tok = strtok(NULL, ","); if (!tok) continue;
        float n2 = (float)atof(tok);

        tok = strtok(NULL, ",");
        float media = tok ? (float)atof(tok) : (n1 + n2) / 2.0f;

        // Encontra aluno pelo RA
        int idx = find_aluno_por_ra(ra);

        if (idx == -1) {
            if (qtdAlunos >= MAX) continue;
            idx = qtdAlunos++;

            strncpy(alunos[idx].ra, ra, MAX_CHAR-1);
            strcpy(alunos[idx].nome, "N/A");
            strcpy(alunos[idx].email, "N/A");
            strcpy(alunos[idx].turma, "N/A");
            alunos[idx].qtdNotas = 0;
        }

        // Insere nota
        if (alunos[idx].qtdNotas < MAX_MATERIAS) {
            int p = alunos[idx].qtdNotas++;
            strncpy(alunos[idx].notas[p].materia, materia, MAX_CHAR-1);

            alunos[idx].notas[p].n1 = n1;
            alunos[idx].notas[p].n2 = n2;
            alunos[idx].notas[p].media = media;
        }
    }

    fclose(f);
    return 1;
}

// Carrega alunos + notas
void carregar_dados() {
    qtdAlunos = 0;
    carregar_alunos_csv();
    carregar_notas_csv();
}

// Salva arquivo alunos.csv
int salvar_alunos_csv() {
    FILE *f = fopen(ARQ_ALUNOS, "w");
    if (!f) { printf("Erro ao abrir %s para escrita\n", ARQ_ALUNOS); return 0; }

    fprintf(f, "Nome,RA,Email,Turma\n");

    for (int i = 0; i < qtdAlunos; i++)
        fprintf(f, "%s,%s,%s,%s\n",
            alunos[i].nome, alunos[i].ra, alunos[i].email, alunos[i].turma);

    fclose(f);
    return 1;
}

// Salva arquivo notas.csv
int salvar_notas_csv() {
    FILE *f = fopen(ARQ_NOTAS, "w");
    if (!f) { printf("Erro ao abrir %s para escrita\n", ARQ_NOTAS); return 0; }

    fprintf(f, "RA,Materia,N1,N2,Media\n");

    for (int i = 0; i < qtdAlunos; i++) {
        for (int j = 0; j < alunos[i].qtdNotas; j++) {
            Nota *n = &alunos[i].notas[j];

            fprintf(f, "%s,%s,%.2f,%.2f,%.2f\n",
                alunos[i].ra, n->materia, n->n1, n->n2, n->media);
        }
    }

    fclose(f);
    return 1;
}

/* ---------- operações do usuário ---------- */

// Mostra boletim completo de um aluno
void mostrar_aluno(const Aluno *a) {
    printf("\n===== DADOS DO ALUNO =====\n");
    printf("Nome : %s\n", a->nome);
    printf("RA   : %s\n", a->ra);
    printf("Email: %s\n", a->email);
    printf("Turma: %s\n", a->turma);

    if (a->qtdNotas == 0) {
        printf("Sem notas cadastradas.\n");
        return;
    }

    printf("\n--- NOTAS ---\n");
    for (int i = 0; i < a->qtdNotas; i++) {
        Nota *n = &a->notas[i];

        printf("%d) %s -> N1: %.2f | N2: %.2f | Média: %.2f\n",
               i + 1, n->materia, n->n1, n->n2, n->media);
    }
}

// Pesquisa boletim pelo RA
void consultar_boletim() {
    char ra[MAX_CHAR];

    printf("\nInforme o RA do aluno: ");
    fgets(ra, sizeof(ra), stdin);
    trim(ra);

    if (ra[0] == '\0') { printf("RA inválido\n"); return; }

    int idx = find_aluno_por_ra(ra);

    if (idx == -1) {
        printf("Aluno não encontrado\n");
        return;
    }

    mostrar_aluno(&alunos[idx]);
}

// Ajusta notas manualmente
void alterar_nota() {
    char ra[MAX_CHAR];

    printf("\nInforme o RA do aluno para alterar nota: ");
    fgets(ra, sizeof(ra), stdin);
    trim(ra);

    if (ra[0] == '\0') { printf("RA inválido\n"); return; }

    int idx = find_aluno_por_ra(ra);

    if (idx == -1) { printf("Aluno não encontrado\n"); return; }
    if (alunos[idx].qtdNotas == 0) { printf("Aluno não possui notas\n"); return; }

    printf("Escolha a matéria a alterar:\n");

    for (int i = 0; i < alunos[idx].qtdNotas; i++)
        printf("%d - %s\n", i+1, alunos[idx].notas[i].materia);

    char buf[32];
    fgets(buf, sizeof(buf), stdin);
    int opc = atoi(buf);

    if (opc < 1 || opc > alunos[idx].qtdNotas) {
        printf("Opção inválida\n");
        return;
    }

    int pos = opc - 1;

    char s1[32], s2[32];
    printf("Nova N1: ");
    fgets(s1, sizeof(s1), stdin);
    trim(s1);

    printf("Nova N2: ");
    fgets(s2, sizeof(s2), stdin);
    trim(s2);

    float n1 = (float)atof(s1);
    float n2 = (float)atof(s2);

    if (!(n1 >= 0 && n1 <= 10 && n2 >= 0 && n2 <= 10)) {
        printf("Notas devem ser entre 0 e 10\n");
        return;
    }

    alunos[idx].notas[pos].n1 = n1;
    alunos[idx].notas[pos].n2 = n2;
    alunos[idx].notas[pos].media = (n1 + n2) / 2.0f;

    if (salvar_notas_csv())
        printf("Nota atualizada e salva\n");
    else
        printf("Nota atualizada na memória, falha ao salvar\n");
}

// Lista alunos por nome
void listar_alunos() {
    printf("\n===== LISTA DE ALUNOS =====\n");

    for (int i = 0; i < qtdAlunos; i++)
        printf("%d - %s (%s)\n", i+1, alunos[i].nome, alunos[i].ra);
}

/* ---------- menu ---------- */

void menu() { 
    while (1) {
        printf("\n========================================\n");
        printf("       SISTEMA ESCOLAR - MENU PRINCIPAL\n");
        printf("========================================\n");
        printf(" 1 - Carregar dados (alunos.csv + notas.csv)\n");
        printf(" 2 - Consultar boletim\n");
        printf(" 3 - Alterar nota\n");
        printf(" 4 - Listar alunos\n");
        printf(" 5 - Salvar alunos.csv\n");
        printf(" 6 - Salvar notas.csv\n");
        printf(" 7 - Sair\n");
        printf("========================================\n");
        printf("Escolha uma opção: ");

        char op[8];
        fgets(op, sizeof(op), stdin);
        int opc = atoi(op);

        switch (opc) {
            case 1:
                carregar_dados();
                printf("Dados carregados. Alunos: %d\n", qtdAlunos);
                break;

            case 2:
                consultar_boletim();
                break;

            case 3:
                alterar_nota();
                break;

            case 4:
                listar_alunos();
                break;

            case 5:
                if (salvar_alunos_csv())
                    printf("alunos.csv salvo\n");
                else
                    printf("falha ao salvar alunos.csv\n");
                break;

            case 6:
                if (salvar_notas_csv())
                    printf("notas.csv salvo\n");
                else
                    printf("falha ao salvar notas.csv\n");
                break;

            case 7:
                printf("Encerrando...\n");
                return;

            default:
                printf("Opção inválida\n");
                break;
        }
    }
}

/* ---------- main ---------- */

int main(void) {
    setlocale(LC_ALL, "pt_BR.UTF-8");
    setlocale(LC_ALL, "");

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    carregar_dados(); // Carrega arquivos ao iniciar
    menu();           // Abre o menu principal
    return 0;
}
