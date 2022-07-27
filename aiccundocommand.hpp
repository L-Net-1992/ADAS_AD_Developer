#ifndef AICCUNDOCOMMAND_H
#define AICCUNDOCOMMAND_H

#include <QUndoCommand>
#include <QUndoStack>
#include "controllers/aiccflowscene.hpp"

class AICCUndoCommand : public QUndoCommand
{
public:
    AICCUndoCommand(AICCFlowScene *scene) :
        _scene(scene),
        _undoStack(QSharedPointer<QUndoStack>(new QUndoStack))
    {

    }

    void undo(){

    }

    void redo(){

    }

private:
    AICCFlowScene *_scene;
    QSharedPointer<QUndoStack> _undoStack;
};

#endif // AICCUNDOCOMMAND_H
