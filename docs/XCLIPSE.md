# ACNHRecomp Xclipse — versão em desenvolvimento

Este fork prepara um aplicativo separado para Android 14 ou superior, inicialmente para o Galaxy S26 Exynos com
Xclipse 960. O [PR de integração](https://github.com/ivanmatheus-dev/ACNHRecomp-Xclipse/pull/1) ainda é um rascunho.
Não há APK experimental publicado neste fork; os artefatos de verificação do Actions são builds sem assinatura e
não são uma versão para instalação.

O aplicativo não contém jogo, saves nem chaves. Prepare a pasta a partir da sua própria cópia com as ferramentas do
projeto original: ela precisa conter `analysis/main.bin`, a biblioteca Android `game/android-arm64/libacnh_game.so`
(ou `libacnh_game.so` no topo), além dos dados e shaders preparados, incluindo `spirv.pak`. Não compartilhe essa pasta.
O aplicativo verifica a compatibilidade da biblioteca com sua ABI e pede outra preparação quando necessário.

Antes de testar uma versão futura, feche o jogo e faça uma cópia dos saves e da pasta preparada em outro local. O pacote
`io.github.ivanmtdev.acnhrecomp` é separado do aplicativo original; confirme qual pasta de jogo está selecionada antes
de copiar ou substituir arquivos. Para instalar uma futura release, baixe o APK assinado na página de Releases deste
fork, confira seu SHA-256 e abra o APK no Android. Só instale atualizações assinadas com a mesma chave.

Na tela inicial, escolha o driver antes de iniciar o jogo. `config.ini` usa `video.gpu_driver`; `NX_GPU_DRIVER` em
`config.txt` tem precedência sobre a escolha da tela e o arquivo de configuração. `auto` tenta RADV quando o driver do
sistema identifica um Xclipse 960; se a tentativa inicial falhar, usa o sistema e informa o motivo. `system` usa o
driver do Android. `turnip` exige Adreno. `radv-xclipse` exige Xclipse 960 detectada e mostra o erro caso a tentativa
inicial falhe, sem trocar silenciosamente. O driver é carregado uma vez pelo processo do jogo: encerre a execução antes
de mudá-lo e inicie novamente. O diagnóstico da última execução aparece na tela inicial; ele mostra GPU, driver efetivo,
versão Vulkan e, quando RADV é carregado, sua revisão. O log também registra a causa da falha inicial.
Com o jogo encerrado, a tela inicial pode programar a limpeza do cache de pipelines da pasta salva para a próxima
execução; os saves e os arquivos de shaders não são apagados.

Na ausência de escolhas anteriores, o perfil Xclipse usa 30 FPS, 720p, FSR 1, geração de quadros desligada, preparação
de pipelines e modo térmico equilibrado. A compilação em segundo plano não pula desenhos por padrão; o runtime espera
quando necessário. O fork não solicita root nem altera limites de clock. O ray tracing não é habilitado; o projeto RADV
informa que ele pode travar a GPU no Xclipse 960.

Os testes locais e de CI ainda não demonstram correção visual, estabilidade ou desempenho no S26. Compare sistema e RADV
com a mesma versão do jogo, save, horário e resolução, incluindo caches frio e aquecido, retomada e uma sessão de
20 minutos; examine folhagens, sombras, transparências, personagem e diálogos. A meta de 30 FPS e os glitches continuam
pendentes de verificação física.

O runtime registra o hash, a etapa e o resultado Vulkan quando a criação de um módulo ou pipeline falha. A verificação
prévia de capacidades SPIR-V cobre apenas um subconjunto (tipos numéricos, geometria, tesselação, interlock e operações
de subgrupo); não substitui a validação completa do shader. `NX_SPIRV_REQUIREMENTS_TRACE=1` em `config.txt` registra
também as capacidades que essa verificação ainda não cobre. Nenhuma capacidade é removida para contornar erros.

O RADV compilado depende da biblioteca privada do sistema Android `libhardware.so`. O hook de carregamento tenta ligar
apenas essa biblioteca ao namespace do driver, sem ativar mapeamento KGSL; a disponibilidade e o carregamento real
ainda precisam ser confirmados no aparelho. Se o carregamento inicial falhar, a escolha explícita mostra o erro.

O restante da documentação Android descreve o aplicativo original e pode mencionar downloads, padrões e opções de root
que não se aplicam a este fork.
