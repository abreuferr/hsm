#: Title : HSM Dinamo
#: Author : "Caio Abreu Ferreira" <cferreira@senhasegura.com>
#: Description : Informações referentes ao HSM da Dinamo
#: Options : None
#: Reference :

# LABORATÓRIO: não registre endereço, usuário ou senha de HSM neste arquivo.
# Use um HSM de teste e informe os valores localmente quando solicitado.

#
# Software Cliente
#

# download
$ wget https://download.dinamonetworks.com/latest/dinamo-4.7.12-1.x64.deb

# instalação
$ sudo dpkg -i dinamo-4.7.12-1.x64.deb

# execução
$ hsmcon <HSM_IP> <HSM_USER>
