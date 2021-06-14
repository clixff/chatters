import { Server } from  "./server";
import SocketsServer from './sockets';
import ChatClient from "./chat";

const server = new Server();

server.setRoutes();

export const chatClient = new ChatClient();

export let socketsServer: SocketsServer | null = null;

if (server.Instance)
{
    socketsServer = new SocketsServer(server.Instance.server);
}

server.listen();
