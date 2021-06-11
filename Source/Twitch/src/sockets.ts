import { Server, Socket } from 'socket.io';
import http from 'http';

export default class SocketsServer
{
    constructor(RawServer: http.Server)
    {
        this.io = new Server(RawServer);
        console.log('[SocketsServer] Sockets server created');

        this.onConnection = this.onConnection.bind(this);
        this.onDisconnect = this.onDisconnect.bind(this);


        this.io.on('connection', this.onConnection);
    }

    onDisconnect(reason: string): void
    {
        this.socket = null;
        console.log(`[SocketsServer] Client disconnected. Reason: ${reason}`);
    }

    onConnection(socket: Socket): void
    {
        if (this.socket)
        {
            console.log('[SocketsServer] Error: Client already connected');
            return;
        }
        
        this.socket = socket;
        console.log('[SocketsServer] Client connected.');

        this.socket.emit('msg');

        this.socket.on('disconnect', this.onDisconnect);
    }


    io?: Server | null;

    /** Singleton client socket instance */
    socket?: Socket | null;
}