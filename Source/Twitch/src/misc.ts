import fetch from "node-fetch";

export interface ITwitchAuthData
{
    bSignedIn: boolean;
    name: string;
    id: number;
}

interface ITwitchAuthInfoResponseError
{
    error?: string,
    status?: number,
    message?: string
}

interface ITwitchAuthInfoResponseOK
{
    data?: Array<{
        id: string,
        login: string,
        display_name: string
    }>;
}

type TwitchAuthInfoResponse = ITwitchAuthInfoResponseError & ITwitchAuthInfoResponseOK;

export function GetTwitchValidDisplayName(login: string, displayName: string): string
{
    return  login.toLowerCase() == displayName.toLowerCase() ? displayName : login;
}

export function getTwitchAppClientID(): string
{
    return 'q7bdrqebiu517umgx01nycj5yu55i4';
}

export function getTwitchAuthData(token: string): Promise<ITwitchAuthData>
{
    return new Promise((resolve) =>
    {
        function parseAuthData(response: TwitchAuthInfoResponse | null)
        {
            const authDataToGame: ITwitchAuthData = {
                bSignedIn: false,
                name: "",
                id: 0
            };

            if (response && !response.error && response.data && response.data.length)
            {
                const authData = response.data[0];
                authDataToGame.bSignedIn = true;
                authDataToGame.name = GetTwitchValidDisplayName(authData.login, authData.display_name);   
                authDataToGame.id = Number(authData.id);

                if (!isFinite(authDataToGame.id))
                {
                    authDataToGame.id = 0;
                }
            }

            resolve(authDataToGame);
        }

        fetch('https://api.twitch.tv/helix/users', {
            method: 'GET',
            headers: 
            {
                'Client-ID': getTwitchAppClientID(),
                'Authorization': `Bearer ${token}`
            }
        }).then(res=>res.json()).then(parseAuthData).catch(() => 
        {
            parseAuthData(null);
        });
    });
}