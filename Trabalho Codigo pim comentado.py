from collections import deque   # Importa deque para manipular fila de alunos
import random                   # Importa biblioteca random para gerar valores aleatórios
import string                   # Importa biblioteca string para usar letras e dígitos
import unicodedata              # Importa unicodedata para remover acentos de nomes

# Dicionários e estruturas principais do sistema
alunos = {}          # Armazena dados dos alunos cadastrados (nome, turma, email)
notas = {}           # Armazena as notas dos alunos por matéria
fila_alunos = deque() # Fila de alunos cadastrados (exemplo de uso com deque)
ras_existentes = set() # Conjunto para armazenar RAs já usados e evitar duplicidade

# --------------------- GERAR RA AUTOMÁTICO ---------------------
def gerar_ra():
    # Gera um RA aleatório de 7 caracteres (letras maiúsculas e números)
    while True:
        ra = ''.join(random.choices(string.ascii_uppercase + string.digits, k=7))
        if ra not in ras_existentes:  # Garante que o RA não se repita
            ras_existentes.add(ra)   # Armazena o RA gerado
            return ra

# --------------------- GERAR EMAIL AUTOMÁTICO ---------------------
def gerar_email(nome):
    # Normaliza o nome removendo acentos e caracteres especiais
    nome_normalizado = unicodedata.normalize('NFD', nome).encode('ascii', 'ignore').decode('utf-8')
    # Divide o nome em partes e cria um email padronizado
    partes = nome_normalizado.lower().split()
    email = ".".join(partes) + "@aluno.com"
    return email

# --------------------- VALIDAR NOTA ---------------------
def validar_nota(valor):
    # Verifica se a nota está entre 0 e 10
    return 0 <= valor <= 10

# --------------------- SALVAR POR MATÉRIA (um arquivo só) ---------------------
def salvar_por_materia(materia):
    # Cria um arquivo com o nome da matéria (ex: matematica.txt)
    arquivo = f"{materia.lower()}.txt"
    ordem_turmas = ["9A", "9B", "8A", "8B"] # Ordem fixa das turmas para exibição

    with open(arquivo, "w", encoding="utf-8") as f:
        # Cabeçalho da tabela
        f.write(f"{'Nome':<20} {'RA':<10} {'Turma':<6} {'N1':<6} {'N2':<6} {'Média':<6}\n")
        f.write("-" * 65 + "\n")

        # Percorre cada turma na ordem definida
        for turma in ordem_turmas:
            f.write(f"\n Turma {turma} \n")
            for ra, mats in notas.items():
                # Verifica se a matéria pertence ao aluno da turma
                if materia in mats and alunos[ra]['turma'] == turma:
                    dados = mats[materia]
                    # Salva as notas e média no arquivo
                    f.write(f"{alunos[ra]['nome']:<20} {ra:<10} {turma:<6} "
                            f"{dados['N1']:<6.2f} {dados['N2']:<6.2f} {dados['Media']:<6.2f}\n")

# --------------------- SALVAR GERAL (reescrevendo) ---------------------
def salvar_geral():
    # Cria/reescreve um arquivo geral com todos os alunos e notas
    arquivo = "alunos_geral.txt"
    ordem_turmas = ["9A", "9B", "8A", "8B"]

    with open(arquivo, "w", encoding="utf-8") as f:
        for turma in ordem_turmas:
            f.write(f"\n Turma {turma} \n")
            for ra, info in alunos.items():
                if info["turma"] != turma:
                    continue

                # Dados principais do aluno
                f.write(f"Nome: {info['nome']}, RA: {ra}, E-mail: {info['email']}, Turma: {info['turma']}\n")

                # Se o aluno já tem notas cadastradas
                if ra in notas and notas[ra]:
                    soma = 0
                    qtd = 0
                    # Escreve todas as matérias com notas
                    for materia, dados in notas[ra].items():
                        f.write(f"{materia:<12} N1: {dados['N1']:.2f}  N2: {dados['N2']:.2f}  Média: {dados['Media']:.2f}\n")
                        soma += dados['Media']
                        qtd += 1
                    media_geral = soma / qtd
                    f.write(f"Média geral: {media_geral:.2f}\n")
                else:
                    f.write("Nenhuma nota cadastrada ainda.\n")

                f.write("-" * 40 + "\n")

# --------------------- FUNÇÃO PRINCIPAL ---------------------
def etapa1():
    # Menu principal do sistema
    print("\n" + "="*50)
    print("      BEM-VINDO AO SISTEMA ESCOLAR")
    print("="*50 + "\n")

    print("Escolha uma opção para executar:\n")
    print("  1 Registrar aluno no sistema")
    print("  2 Cadastrar notas e medias por matéria")
    print("  3 Consultar boletim completo")
    print("  4 Alterar nota do aluno")
    print("  5 Pesquisar aluno por RA")
    print("  6 Sair do sistema\n")
    print("="*50)

    opcao = input("Opção: ").strip()
    if not opcao.isdigit(): # Valida se o usuário digitou número
        print("\nPor favor, digite apenas números!\n")
        return

    # --------------------- REGISTRO DE ALUNOS ---------------------
    if opcao == "1":
        # Cadastro de um novo aluno
        while True:
            print("\n--- Registro de aluno no sistema ---")
            nome = input("Digite o nome do aluno: ").strip().capitalize()
            if not nome.replace(" ", "").isalpha(): # Apenas letras
                print("\nPor favor, digite apenas letras.\n")
                continue

            turma = input("Digite a turma do aluno (8A, 8B, 9A, 9B): ").strip().upper()
            if turma not in ["8A", "8B", "9A", "9B"]: # Validação da turma
                print("\nTurma inválida! Digite apenas 8A, 8B, 9A ou 9B.\n")
                continue

            ra = gerar_ra()         # Gera RA único
            email = gerar_email(nome) # Gera email automático

            # Armazena no dicionário
            alunos[ra] = {"nome": nome, "turma": turma, "email": email}
            fila_alunos.append(ra)

            # Confirmação do cadastro
            print("\n-----------------------\n")
            print(f"Aluno cadastrado com sucesso!")
            print(f"Aluno: {nome}")
            print(f"RA: {ra}")
            print(f"Turma: {turma}")
            print(f"E-mail: {email}")
            print("-----------------------\n")
            break

    # --------------------- REGISTRO DE NOTAS ---------------------
    elif opcao == "2":
        # Cadastro de notas por matéria
        ra = input("Digite o RA do aluno: ").strip().upper()
        if ra not in alunos:  # Verifica se aluno existe
            print("\nAluno não encontrado! Cadastre primeiro.\n")
            return

        # Escolha da matéria
        print("\n" + "="*40)
        print("     CADASTRO DE NOTAS POR MATÉRIA")
        print("="*40 + "\n")
        print("Escolha a matéria:\n")
        print("  [1] Matemática")
        print("  [2] Português")
        print("  [3] Ciências")
        print("  [4] História\n")
        print("="*40)

        materia_input = input("Matéria (número): ").strip()
        materias = {1: "Matemática", 2: "Português", 3: "Ciências", 4: "História"}

        if not materia_input.isdigit() or int(materia_input) not in materias:
            print("\nOpção inválida!\n")
            return

        materia = materias[int(materia_input)]

        # Entrada das notas
        try:
            n1 = float(input("Nota N1: "))
            n2 = float(input("Nota N2: "))
        except ValueError:
            print("\nDigite apenas números para as notas.\n")
            return

        # Valida notas
        if not (validar_nota(n1) and validar_nota(n2)):
            print("\nNotas inválidas! Digite valores entre 0 e 10.\n")
            return

        media = (n1 + n2) / 2

        # Salva notas no dicionário
        if ra not in notas:
            notas[ra] = {}
        notas[ra][materia] = {"N1": n1, "N2": n2, "Media": media}

        salvar_por_materia(materia)   # Atualiza o arquivo da matéria
        salvar_geral()                # Atualiza o arquivo geral

        print(f"Sua média em {materia} é: {media:.2f}")

    # --------------------- BOLETIM DO ALUNO ---------------------
    elif opcao == "3":
        # Consultar boletim do aluno
        ra = input("Digite o RA do aluno: ").strip().upper()
        if ra not in alunos:
            print("\nAluno não encontrado!\n")
            return

        # Exibe boletim
        print("\n" + "="*40)
        print(f"      BOLETIM DE {alunos[ra]['nome'].upper()}")
        print("="*40 + "\n")
        print(f"Nome: {alunos[ra]['nome']}")
        print(f"RA: {ra}")
        print(f"Turma: {alunos[ra]['turma']}")
        print(f"E-mail: {alunos[ra]['email']}")
        print("---------------------------\n")

        if ra in notas and notas[ra]:
            soma_geral = 0
            qtd_materias = 0
            # Lista todas as matérias do aluno
            for materia, dados in notas[ra].items():
                print(f"{materia:<12} N1: {dados['N1']:.2f}  N2: {dados['N2']:.2f}  Média: {dados['Media']:.2f}")
                soma_geral += dados['Media']
                qtd_materias += 1
            media_geral = soma_geral / qtd_materias
            status = "APROVADO" if media_geral >= 6 else "REPROVADO"
            print("\n---------------------------")
            print(f"Média geral: {media_geral:.2f}")
            print(f"Status: {status}")
            print("---------------------------\n")
        else:
            print("Nenhuma nota cadastrada ainda.")

    # --------------------- ALTERAR NOTA ---------------------
    elif opcao == "4":
        # Alteração de notas para alunos reprovados
        ra = input("Digite o RA do aluno: ").strip().upper()
        if ra not in alunos:
            print("\nAluno não encontrado!\n")
            return

        if ra not in notas or not notas[ra]:
            print("\nEsse aluno não possui notas cadastradas.\n")
            return

        soma_geral = sum(dados['Media'] for dados in notas[ra].values())
        qtd_materias = len(notas[ra])
        media_geral = soma_geral / qtd_materias

        if media_geral >= 6:
            print("\nEsse aluno já está APROVADO.\n")
            return

        # Escolher a matéria a alterar
        print("\n--- Alteração de nota ---")
        for i, materia in enumerate(notas[ra].keys(), 1):
            print(f"[{i}] {materia}")

        materia_input = input("Escolha a matéria para alterar nota: ").strip()
        try:
            materia_selecionada = list(notas[ra].keys())[int(materia_input) - 1]
        except (ValueError, IndexError):
            print("\nOpção inválida!\n")
            return

        # Entrada das novas notas
        try:
            n1 = float(input(f"Digite a nova nota N1 de {materia_selecionada}: "))
            n2 = float(input(f"Digite a nova nota N2 de {materia_selecionada}: "))
        except ValueError:
            print("\nDigite apenas números!\n")
            return

        if not (validar_nota(n1) and validar_nota(n2)):
            print("\nNotas inválidas! Digite valores entre 0 e 10.\n")
            return

        # Atualiza notas no dicionário
        media = (n1 + n2) / 2
        notas[ra][materia_selecionada] = {"N1": n1, "N2": n2, "Media": media}

        salvar_por_materia(materia_selecionada)
        salvar_geral()
        print(f"\nNota de {materia_selecionada} alterada com sucesso!\n")

    # --------------------- PESQUISAR POR RA ---------------------
    elif opcao == "5":
        # Pesquisa rápida de aluno pelo RA
        ra = input("Digite o RA do aluno para pesquisar: ").strip().upper()
        if ra not in alunos:
            print("\nAluno não encontrado!\n")
            return

        info = alunos[ra]
        # Exibe dados básicos do aluno
        print("\n--- Dados do Aluno ---")
        print(f"Nome: {info['nome']}")
        print(f"RA: {ra}")
        print(f"Turma: {info['turma']}")
        print(f"E-mail: {info['email']}\n")

        # Exibe notas se houver
        if ra in notas and notas[ra]:
            print("Notas:")
            for materia, dados in notas[ra].items():
                print(f"{materia:<12} N1: {dados['N1']:.2f}  N2: {dados['N2']:.2f}  Média: {dados['Media']:.2f}")
        else:
            print("Sem notas cadastradas.")

    # --------------------- SAIR ---------------------
    elif opcao == "6":
        # Finaliza o sistema
        print("\nSaindo do sistema...")

        # Resetando banco de dados em memória
        alunos.clear()
        notas.clear()
        fila_alunos.clear()
        ras_existentes.clear()

        return False

    else:
        print("\nOpção inválida!")

# --------------------- LOOP PRINCIPAL ---------------------
# Mantém o sistema em execução até que o usuário escolha sair
while True:
    if etapa1() == False:
        break