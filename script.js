// Configurações do ThingSpeak
const channelID = '2840207'; // Channel ID
const readAPIKey = '5UWNQD21RD2A7QHG'; // Read API Key
const writeAPIKey = '9NG6QLIN8UXLE2AH'; // Write API Key
const temperatureField = 'field1';
const levelField = 'field2';
const bombaField = 'field3';
const resistenciaField = 'field4';
const modoAutomaticoField = 'field5';
const temperaturaAlvoField = 'field6'; 
const tempoLigaField = 'field7';
const tempoDesligaField = 'field8';

// Elementos da seção de controle manual
const temperatureElement = document.getElementById('temperature');
const levelElement = document.getElementById('level');
const bombaOnButton = document.getElementById('bombaOn');
const bombaOffButton = document.getElementById('bombaOff');
const resistenciaOnButton = document.getElementById('resistenciaOn');
const resistenciaOffButton = document.getElementById('resistenciaOff');

// Variáveis para armazenar os últimos valores válidos do thingspeak
let lastValidTemperature = '--';
let lastValidLevel = '--';

// Função para buscar dados do ThingSpeak em tempo real
function fetchData() {
    fetch(`https://api.thingspeak.com/channels/${channelID}/feeds/last.json?api_key=${readAPIKey}`)
        .then(response => {
            if (!response.ok) {
                throw new Error('Erro na requisição: ' + response.statusText);
            }
            return response.json();
        })
        .then(data => {
            console.log('Dados recebidos:', data); // Log dos dados recebidos

            // Atualiza a temperatura se o campo não estiver vazio
            if (data[temperatureField] && data[temperatureField].trim() !== '') {
                lastValidTemperature = data[temperatureField];
            }
            temperatureElement.textContent = lastValidTemperature;

            // Atualiza o nível se o campo não estiver vazio
            if (data[levelField] && data[levelField].trim() !== '') {
                lastValidLevel = data[levelField];
            }
            levelElement.textContent = lastValidLevel;

            // Atualiza o texto do status da bomba baseado no field3 do thingspeak
            // Está com problema... assim que o thingspeak atualiza, o texto fica OK, mas depois de algum tempo muda para desligado sozinho, mesamo com o field em 1
            if (data[bombaField] == 1) {
                statusBomba.textContent = 'Bomba Ligada'; // Atualiza o texto do status da bomba
                statusBomba.style.color = 'green'; // Muda a cor do texto para verde
            } else {
                statusBomba.textContent = 'Bomba Desligada'; // Atualiza o texto
                statusBomba.style.color = 'red'; // Muda a cor do texto para vermelho
            }
            
            // Atualiza o texto do status do aquecedor baseado no field4 do thingspeak
            if (data[resistenciaField] == 1) {
                statusAquecedor.textContent = 'Aquecedor Ligado'; // Atualiza o texto do status da bomba
                statusAquecedor.style.color = 'green'; // Muda a cor do texto para verde
            } else {
                statusAquecedor.textContent = 'Aquecedor Desligado'; // Atualiza o texto
                statusAquecedor.style.color = 'red'; // Muda a cor do texto para vermelho
            }

        })
        .catch(error => {
            console.error('Erro ao buscar dados:', error);
            // Mantém os últimos valores válidos em caso de erro
            temperatureElement.textContent = lastValidTemperature;
            levelElement.textContent = lastValidLevel;
        });
}

// Atualiza os dados em tempo real a cada 5 segundos
if (temperatureElement && levelElement) {
    setInterval(fetchData, 5000);
    fetchData(); // Busca os dados imediatamente ao carregar a página
}


// Função para atualizar um campo no ThingSpeak
function updateField(field, value) {
    const url = `https://api.thingspeak.com/update?api_key=${writeAPIKey}&${field}=${value}`;
    console.log(`Enviando requisição para: ${url}`); // Log da URL de requisição

    fetch(url, {
        method: 'POST'
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Erro na requisição: ' + response.statusText);
        }
        return response.text();
    })
    .then(data => {
        console.log(`Campo ${field} atualizado com sucesso. Resposta: ${data}`);
    })
    .catch(error => {
        console.error(`Erro ao atualizar campo ${field}:`, error);
    });
}

// Event listeners para os botões do controle manual
if (bombaOnButton && bombaOffButton && resistenciaOnButton && resistenciaOffButton) {
    bombaOnButton.addEventListener('click', () => {
        if (modoAutomatico === 0) { // Verifica se o modo automático está desligado
            console.log('Ligando bomba...');
            updateField(bombaField, 1);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });

    bombaOffButton.addEventListener('click', () => {
        if (modoAutomatico === 0) { // Verifica se o modo automático está desligado
            console.log('Desligando bomba...');
            updateField(bombaField, 0);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });

    resistenciaOnButton.addEventListener('click', () => {
        if (modoAutomatico === 0) { // Verifica se o modo automático está desligado
            console.log('Ligando resistência...');
            updateField(resistenciaField, 1);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });

    resistenciaOffButton.addEventListener('click', () => {
        if (modoAutomatico === 0) { // Verifica se o modo automático está desligado
            console.log('Desligando resistência...');
            updateField(resistenciaField, 0);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });
}


// Incluído para o modo automático
// Função para dados controle automático
let modoAutomatico = 0; // 0 para desligado, 1 para ligado

function toggleModoAutomatico() {
    modoAutomatico = modoAutomatico === 0 ? 1 : 0;
    const botao = document.getElementById('modoAutomatico');
    botao.textContent = modoAutomatico === 1 ? 'Ligado' : 'Desligado';   

    if (modoAutomatico === 1) { // Verifica se o modo automático está ligado
        // Atualiza os campos 5, 6, 7 e 8 no ThingSpeak - não está funcionando , implementar botão ou enter para envio individual
        updateField(modoAutomaticoField, 1);
        //updateField(tempoLigaField, Number(tempoLiga);
        //updateField(tempoDesligaField, Number(tempoDesliga);
        //updateField(temperaturaAlvoField, Number(temperaturaAlvo);
    } else {
        updateField(modoAutomaticoField, 0);
    }

}






//document.getElementById('bombaForm').addEventListener('submit', function(event) {
    //event.preventDefault();

    //const tempoLiga = parseInt(document.getElementById('tempoLiga').value);
    //const tempoDesliga = parseInt(document.getElementById('tempoDesliga').value);
    //const temperaturaAlvo = parseInt(document.getElementById('temperaturaAlvo').value);

//});


